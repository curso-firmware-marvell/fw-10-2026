# TP4 — Bootloader y arranque del firmware

En este práctico vas a escribir un bootloader chico y usarlo para arrancar el
firmware del práctico anterior, todo en simulación. El BL arranca primero, hace
lo mínimo y le pasa el control al FW cargado, que tiene que correr igual que
antes.

Se apoya en lo que ya hiciste:

- **TP1:** el entorno (toolchain en `/opt/riscv`, Verilator).
- **TP2:** la plataforma, `fw/tests/test_fw_status.c`, los parámetros de `top.v` y `REG_FW_STATUS`.
- **TP3:** Makefile, enlazado e inspección con binutils (`nm`, `objdump`, `readelf`, `size`). El TP4 incluye su propia implementación CRC en `fw/lib/src/strutils.c`, independiente del material del TP3.

---

## Requisitos

- **El FW anterior compilando y simulando** (TP1 y TP2). Desde la raíz del repositorio:

  ```bash
  make build_project_config                                   # genera fw/config.h
  make -C fw clean
  make -C fw STANDALONE=1 APP_SRC="print.c test_fw_status.c"
  make sim                                                    # termina con REGISTER_FW_STATUS = 000000ff
  ```

  `STANDALONE=1` conserva el flujo anterior, sin bootloader. El flujo TP4 combinado
  se compila con `make -C fw` y es el que instala la imagen por defecto.

---

## Punto de partida

Los archivos del TP4 se integran en el firmware de la plataforma, dentro de `fw/`:

```
fw/tests/
├── test_adc.c
├── test_data_bss.c
├── test_dma.c
├── test_fw_status.c
├── test_fw_status.hex
└── test_new_uart.c

fw/
├── Makefile             # flujo combinado y opción STANDALONE=1
├── README.md            # esta consigna
├── memory_map.ld
├── fw_image.h           # formato del header del FW (no lo modifiques)
├── lib/
│   ├── inc/
│   │   ├── includes.h   # registros y API de impresión
│   │   └── strutils.h   # API CRC local
│   └── src/
│       ├── print.c     # mensajes por UART
│       └── strutils.c  # implementación CRC local, reemplazable
├── bootloader/          # bl_start.S, bl_main.c, bl.ld -> completar
├── app/                 # app_startup.S, app.ld -> completar
└── tools/
    ├── mkimage.py       # firma el FW (CRC) y arma la imagen BL + FW
    └── check_load.py    # compara fw.hex con lo que cargó el simulador (hito 8)
```

El TP usa cosas de otras carpetas del repo, sin copiarlas:

| De dónde | Qué | Para qué |
|---|---|---|
| `fw/lib/src/` | `print.c` | Los mensajes por UART del BL y del FW |
| `fw/lib/inc/` | `includes.h` | Registros y API de impresión |
| `fw/` | `config.h` | Configuración de registros |
| `fw/tests/` | Tests del FW, incluidos `test_fw_status.c` y `test_data_bss.c` | Tests del FW anterior y del arranque |
| `fw/` | `startup.S`, `linker.ld` | El arranque standalone anterior, para comparar (hitos 1 y 6) |
| Raíz del repositorio | `make sim`, `rtl/testbench.sv` | La simulación, que carga `fw/fw.hex` |
| `fw/lib/` | `src/strutils.c`, `inc/strutils.h` | Implementación local de `str_crc8()` para validar la imagen (hito 7); podés reemplazarla |

Trabajá directamente en `fw/`. Para ver lo que falta:

```bash
grep -rn TODO .
make
```

- Los archivos se entregan compilando. Hasta que completes el header del FW, `mkimage.py` frena con un error.
- No modifiques el RTL ni la configuración de la plataforma. El build instala la imagen generada en `fw/fw.hex`, que es el archivo que lee el testbench.
- Los `.c` del FW (`fw/lib/src/print.c`, `fw/tests/test_fw_status.c`) tampoco se modifican.

---

## Especificación

### La plataforma, mirada desde el arranque

| Qué | Valor | Dónde mirarlo |
|---|---|---|
| Vector de reset | `PROGADDR_RESET` = `0x0000_0000` | parámetros de picorv32 en `rtl/top.v` (TP2) |
| Vector de IRQ | `PROGADDR_IRQ` = `0x0000_0010`; `ENABLE_IRQ = 0` | idem |
| SP al salir de reset | No se inicializa (`STACKADDR` = `0xFFFF_FFFF`) | idem |
| Memoria de instrucciones | `0x0000_0000`–`0x0001_FFFF` (128 KiB) | `project_config.def` |
| RAM | `0x0002_0000`–`0x0003_FFFF` (128 KiB) | idem |
| Carga del FW | `init_memory()` lee `fw/fw.hex`, una palabra por línea, desde la palabra 0 | `rtl/testbench.sv` |
| Fin de la simulación | `REG_FW_STATUS = 0xFF`, o el core hace trap | idem (actividad extra del TP2) |
| `ebreak` / instrucción ilegal | El core hace trap (`ENABLE_IRQ = 0`, `CATCH_ILLINSN = 1`) | `top.v` |

Como el testbench carga **un solo** `fw.hex`, el BL y el FW tienen que ir juntos en
esa imagen.

Para ver qué pasa usá lo mismo que en el TP2:

- **`print_str()` / `print_dec()`** de `fw/lib/src/print.c`: el testbench imprime cada mensaje como `UART: …`.
- **`REG_FW_REPORT`** como marca de avance: no se imprime, pero se ve en las ondas con `make wave`.

### Mapa de memoria del TP (`memory_map.ld`)

| Región | Desde | Tamaño | Qué va |
|---|---|---|---|
| `BL_ROM` | `0x0000_0000` | 4 KiB | bootloader (su vector table en `0x0`) |
| `APP_ROM` | `0x0000_1000` | 124 KiB | FW del TP anterior: header + vector table + código |
| `RAM` | `0x0002_0000` | 128 KiB | `.data`, `.bss` y stack |

### Header + vector table del FW (`fw_image.h`)

| Offset | Campo | Quién lo completa |
|---|---|---|
| `0x00` | `magic` = `0x46345054` ("TP4F") | el arranque del FW |
| `0x04` | `load_addr`: dirección de linkeo del FW | el linker |
| `0x08` | `image_size`: bytes desde `load_addr` hasta el final de la imagen | el linker |
| `0x0C` | `image_crc`: CRC-8 (`str_crc8`) de `0x10` hasta el final | `tools/mkimage.py` |
| `0x10` | `initial_sp` | el linker |
| `0x14` | `reset_handler` (alineado a 4) | el linker |

---

## Hitos

Completá los hitos en orden.

### Hito 1 — Reconocimiento del arranque actual

**Objetivo:** ver qué hace el flujo standalone anterior entre el reset y `main()`.

Compilá el FW anterior con `STANDALONE=1`, que selecciona el arranque original sin
bootloader:

```bash
make -C fw clean
make -C fw STANDALONE=1 APP_SRC="print.c test_fw_status.c"
```

Respondé en el informe:

1. ¿Qué instrucción hay en la línea 1 de `fw.hex`? Buscala en `riscv64-unknown-elf-objdump -d build/standalone.elf`. ¿Por qué `_start` queda en `0x0`? ¿Qué pasaría si `print.c` se linkeara antes que `startup.S`?
2. Volvé a linkear agregando `-Wl,--orphan-handling=warn`:

   ```bash
   make clean && make STANDALONE=1 APP_SRC="print.c test_fw_status.c" LDFLAGS="-Wl,--gc-sections -Wl,--orphan-handling=warn"
   ```

   ¿Qué secciones no ubica `linker.ld`? ¿En qué sección terminan `int_0`, `int_1` e `int_2`?
3. Compará `startup.S` con la diapo "De 0 a main": ¿qué pasos faltan? ¿Por qué el test anda igual en simulación?

### Hito 2 — Vector table e init mínimo del BL

**Objetivo:** que el BL sea lo primero que ejecuta el core.

1. En `bootloader/bl_start.S` armá la vector table:
   - en `0x00`, un salto a `bl_reset_handler`;
   - en `0x10` (`PROGADDR_IRQ`), un salto a `bl_unexpected`.
2. En `bl_reset_handler` cargá `sp` y llamá a `bl_main()`.
3. En `bootloader/bl.ld`:
   - ubicá la vector table primero;
   - definí `_bl_stack_top`;
   - hacé que el link falle si el BL usa variables globales o si la vector table no queda en `0x0`.

**Comprobá:**

```bash
riscv64-unknown-elf-readelf -h build/bl.elf | grep Entry       # 0x0
riscv64-unknown-elf-objdump -d build/bl.elf | head -20         # la vector table en 0x00..0x10
```

### Hito 3 — El FW del TP anterior relinkeado en `0x1000`

**Objetivo:** que el FW quede donde el BL lo espera, con `.data` y `.bss` donde dice el
linker script (diapo 4).

Completá `app/app.ld`:

1. `__fw_start` y la sección `.fw_header` primero, al inicio de `APP_ROM`.
2. `.text` y `.rodata` (incluida `.srodata`).
3. `.data` con VMA en RAM y LMA dentro de la imagen, con `_sdata`, `_edata` y `_sidata`; incluí `.sdata`.
4. `.bss` con `_sbss` y `_ebss`; incluí `.sbss` y `COMMON`.
5. `__fw_end`, `__fw_size` y `_stack_top`.

**Comprobá:** `riscv64-unknown-elf-readelf -S -l build/app.elf` (`.data` con VirtAddr en
RAM y PhysAddr dentro de `APP_ROM`) y `riscv64-unknown-elf-nm -n build/app.elf`.

### Hito 4 — Header y arranque del FW ("De 0 a main")

**Objetivo:** escribir el start-up completo de la diapo.

En `app/app_startup.S`:

1. Completá los seis campos del header según `fw_image.h`.
2. Escribí el `Reset_Handler`:
   - cargar `sp`;
   - copiar `.data` desde `_sidata`;
   - poner `.bss` en cero;
   - llamar a `main`;
   - si `main` vuelve sin haber dejado `REG_FW_STATUS = 0xFF`, ejecutar `ebreak`.

**Comprobá:** `make` llega hasta el final y `mkimage.py` muestra el header con
`reset_handler=0x00001018`.

### Hito 5 — El salto del BL al FW

**Objetivo:** pasarle el control al FW respetando la ABI (diapo de ABI).

1. En `bl_jump()` cargá el `sp` que pide el FW y saltá a su `Reset_Handler`. Tené en cuenta en qué registros llegan los argumentos según la ABI ILP32.
2. En `bl_main()`:
   - avisá por UART que arrancó y dejá `REG_FW_REPORT` en `BL_REPORT_START`;
   - leé `initial_sp` y `reset_handler` del header;
   - saltá.

**Comprobá:**

```bash
make sim
```

En la consola tienen que aparecer los mensajes del BL seguidos por la salida de
`test_fw_status.c`, y al final `REGISTER_FW_STATUS = 000000ff`. Ejecutá `make sim`
desde `fw/` (o `make -C fw sim` desde la raíz): el build integrado instala la
imagen BL + FW en `fw/fw.hex` antes de iniciar la simulación. No ejecutes el
`all` de la raíz para probar `STANDALONE=1`, porque ese target construye el flujo
combinado por defecto.

### Hito 6 — Verificar `.data` y `.bss`

**Objetivo:** comprobar que el arranque del FW hace lo que dice la diapo.

```bash
make sim APP_SRC="print.c test_data_bss.c"                # PASS
make sim APP_SRC="print.c test_data_bss.c" STANDALONE=1   # FAIL (arranque original)
```

Explicá en el informe por qué con el arranque original falla.

### Hito 7 — Validar la imagen antes de saltar

**Objetivo:** que el BL no salte a una imagen inválida. Es la misma idea que la trama
del hito 10 del TP3 (byte de sincronismo + CRC-8), con un header más completo.

1. Usá la implementación local `fw/lib/src/strutils.c` o reemplazala por la tuya; el `Makefile` la compila para el BL. Su API está en `fw/lib/inc/strutils.h`. `tools/mkimage.py` calcula el mismo CRC (polinomio `0x07`, inicial `STR_CRC8_INIT = 0xFF`). Si tu `str_crc8()` usa algo de la biblioteca estándar, el link del BL falla: no hay libc.
2. Antes de saltar, verificá en orden:
   1. `magic`;
   2. `load_addr` igual al slot;
   3. `image_size` dentro del slot;
   4. CRC;
   5. `initial_sp` dentro de la RAM y alineado a 16;
   6. `reset_handler` dentro de la imagen.
3. Implementá `bl_fail()`: mensaje por UART, `REG_FW_REPORT = BL_REPORT_ERROR | código`, `REG_FW_STATUS = 0xBAD` y `bl_halt()`.

**Comprobá:**

```bash
make sim CORRUPT=1     # mkimage invierte un bit del FW después de firmarlo
```

El BL tiene que informar el error de CRC y la simulación tiene que terminar con trap.

### Hito 8 — Revisar la memoria y matchear con el HEX (diapo 12)

**Objetivo:** cerrar el círculo entre el linker script, el `.hex` y lo que el simulador
cargó de verdad.

| Paso de la diapo | Qué mostrar en el informe |
|---|---|
| 1. Abrir el `.hex` / `.map` | Línea N de `build/fw.hex` = dirección `4·(N−1)`. Ubicá el BL (línea 1) y el header del FW (línea 1025). Mirá `build/bl.map` y `build/app.map` |
| 2. Inspeccionar con readelf / objdump / GDB | `objdump -d` de la vector table del BL; `objdump -s` del header del FW |
| 3. Vector table → `Reset_Handler` | La palabra en `0x1014` coincide con `nm` y con el *Entry point* de `readelf -h build/app.elf` |
| 4. `.data` y `.bss` donde dice el linker script | `readelf -S -l`, `nm` (`_sidata`, `_sdata`, `_ebss`…) y lo que imprime `fw/tests/test_data_bss.c` |
| 5. Comparar el `.hex` con lo que cargó el simulador | `make check` con **RESULTADO: OK** |

Ojo: el log del simulador imprime `loading: <i> -> <dato>`, y `<i>` es el índice de
**palabra**, no la dirección. `make inspect` junta los comandos de los pasos 2 a 4.

### Hito 9 — Informe

Respondé:

1. ¿Por qué la vector table del BL son instrucciones y la de Cortex-M de la diapo son direcciones?
2. ¿Qué hace cada paso del `Makefile`?
   - ¿Por qué `bl.elf` y `app.elf` se linkean por separado?
   - ¿Qué dirección usa `objcopy -O binary` para `.data`?
   - ¿Qué agrega `mkimage.py` entre el BL y el FW?
3. ¿Por qué la palabra en `0x100C` no coincide entre `app.elf` y `fw.hex`?
4. Compará el FW de `fw.hex` (desde la línea 1025) con el `fw.hex` del TP anterior: ¿qué instrucciones cambian al relinkear en `0x1000` y cuáles no?
5. ¿Por qué el salto al FW se hace sin link (`jr`) y no con `jalr ra`? ¿Por qué el FW vuelve a cargar `sp` si el BL ya se lo dejó?
6. ¿Por qué el BL no puede dejar `REG_FW_STATUS` en `0xFF`? ¿Y por qué en un error no alcanza con escribir `0xBAD`?
7. `fw/lib/src/strutils.c` se compila para RISC-V sin libc. ¿Qué funciones de `strutils.o` quedan en `bl.elf` y por qué (mirá `nm` y `-Wl,--gc-sections`)?
8. En el TP2 compilaste con `rv32ic` y `COMPRESSED_ISA(1)`. Si compilás el BL así, ¿qué le pasaría a la vector table sin el `.option norvc` de `bl_start.S`?

### Extra (opcional)

- **Costo del CRC:** con los ciclos totales de la simulación (como en la tabla del TP2), medí cuántos ciclos por byte cuesta tu `str_crc8()` en el PicoRV32. ¿Cambia con `-march=rv32im` y `ENABLE_MUL(1)`? ¿Por qué?

---

## Entrega

1. Verificá:

   ```bash
   make sim && make check
   ```

   `make check` tiene que terminar con `RESULTADO: OK`.
2. Entregá el directorio `fw/` completo (sin ningún `TODO` pendiente), el log de `make sim` y el de `make sim CORRUPT=1` (`build/sim.log`), y el informe con las respuestas de los hitos 1, 6, 8 y 9.

---

## Problemas frecuentes

| Síntoma | Causa probable |
|---|---|
| `mkimage: ... más chico que el header` | `.fw_header` no está primero en `app.ld`, o falta `KEEP` (`--gc-sections` la descarta) |
| `mkimage: magic ...` | Header incompleto en `app_startup.S` |
| `image_size != tamaño de app.bin` | Secciones fuera de las que cubre `__fw_size` (revisá `.sdata`, `.srodata`, `.sbss` con `--orphan-handling=warn`) |
| La simulación no termina nunca | No se carga `sp` en el BL, o se escribe `0xBAD` sin `ebreak` |
| `TRAP!` apenas arranca | La vector table no quedó en `0x0`, o el BL salta a una dirección inválida |
| Error de CRC con la imagen sana | No se calcula desde `0x10` hasta `image_size`, o `str_crc8()` no da `0x32` para `{0x01, 0x02, 0x03, 0x04}` (ejemplo del TP3) |
| `undefined reference` al linkear el BL | Tu `str_crc8()` usa la biblioteca estándar: en el BL no hay libc |
| ASSERT del BL por `.data`/`.bss` | Alguna variable global (por ejemplo en `strutils.c`); una tabla constante tiene que ser `const` |
| Globales con valor inicial en 0 | No se copia `.data`, o `.sdata` quedó fuera de `_sdata`…`_edata` |
| Sale el FW solo, sin el BL | Se generó la imagen con `STANDALONE=1`; volvé a ejecutar `make` desde `fw/` para generar la imagen combinada |
