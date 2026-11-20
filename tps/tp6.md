# TP6 — UART1 y Procesador de Comandos

El TP se resuelve en **dos hitos** progresivos. Cada hito tiene su propio test, que se corre con un `make test_...` desde la raíz del repositorio y dice si lo resolviste.

| Hito | Qué hacés | Test y qué verifica |
|---|---|---|
| **1 — Control de flujo** | Bajar RTS cuando el FIFO RX está cerca de llenarse y volver a subirlo cuando se vació, para soportar una ráfaga continua. | `make test_flow_control`: una ráfaga de 1024 bytes contra un consumidor lento: respuesta correcta y **cero bytes perdidos**. |
| **2 — Procesador de comandos** | Implementar el procesador de comandos del protocolo (sección 3). | `make test_main`: tramas correctas, errores de parámetros, validación de direcciones, resincronización. |

---

## 0. Paso 0: lo que tienen que traer del TP5

El TP6 se apoya en el TP5. **Antes de empezar los hitos tienen que tener implementado todo lo del TP5 y estos dos tests pasando** (desde la raíz del repositorio):

```bash
make test_drivers        # drivers de LED, UART0, DMA y ADC: tiene que terminar en [ OK ] PASS
make test_uart_serial    # driver de UART1 (hito 3.b del TP5): tiene que terminar en [ OK ] PASS
```

`test_drivers` es el chequeo de la cátedra sobre sus drivers (ver la [consigna del TP5](tp5/tp5.md) para la interfaz exacta que exige): mientras falte un archivo o una función ni siquiera compila; cuando compila y algo no cumple, imprime códigos `Tnn` sin explicar el motivo. `test_uart_serial` prueba el driver de la UART1 (`uart1_init`, `uart1_getc`, `uart1_putc`, ...) con un *host* que manda y espera bytes por la línea serie. Si alguno de los dos no pasa, resuélvanlo antes de seguir con este TP.

Qué usa cada hito de lo que ya tienen:

- El **control de flujo** (hito 1) completa el driver de la **UART1**: la función `uart1_set_rts()` quedó pendiente en el TP5.
- El **procesador de comandos** (hito 2) recibe y responde por el driver de la **UART1**, y ejecuta cada comando con los drivers de **ADC, DMA, LED y UART0**.

Recuerden dónde va cada cosa: los `.h` en `fw/drivers/inc/`, los `.c` en `fw/drivers/src/`, y cada `.c` se suma a `fw/sources.mk` (incluido `drivers/src/uart1.c`).

**Cómo traer el código del TP6.** Esta rama (`tp6`) parte de la del TP5. Desde su repositorio, con el TP5 ya commiteado:

```bash
git pull --no-rebase origin tp6
```

(`--no-rebase` hace que el pull sea un merge. Sin esa opción, Git se queja cuando su rama y `tp6` tienen commits distintos.)

Si hicieron la demo del TP5 en `fw/app/main.c`, el pull va a marcar un **conflicto en ese archivo**: el TP6 trae ahí el esqueleto del procesador de comandos. Quédense con la versión del TP6 y terminen el merge:

```bash
git checkout --theirs fw/app/main.c
git add fw/app/main.c
git commit
```

Su demo del TP5 no se pierde: queda en el historial de git (`git log -p -- fw/app/main.c`). Si quieren seguir pudiéndola correr, antes del pull copien su `fw/app/main.c` a `fw/app/demo_tp5.c` (y commiteen la copia): se compila con `make -C fw TARGET=demo_tp5`.

---

## 1. Contexto

En los prácticos anteriores desarrollaron drivers en C para los periféricos de la plataforma usando los registros de `fw/config.h`. Acá van a completar primero el driver de la **UART1** con el control de flujo, y después a integrar los drivers detrás de un **procesador de comandos**: un módulo de software que recibe un flujo continuo de bytes por UART1, reconoce los paquetes válidos, ejecuta el comando de cada uno y devuelve una respuesta por la misma UART.

**Cómo resuelvan el diseño interno (driver, sincronización de trama, decodificación, despacho de comandos) queda a su criterio.** Se evalúa la robustez, claridad y mantenibilidad de la solución.

### 1.1 La UART1

UART1 es una **UART serie real**: 8N1, tiempo de bit configurable (`BAUD_DIV`), FIFOs de 16 bytes en recepción y transmisión, flags de overrun y de framing error, y líneas RTS/CTS. A diferencia de UART0 —un buffer de bytes que usa `print.c`— acá los bytes entran y salen de a uno, bit a bit, y si no se leen a tiempo se pierden.

El driver básico de la UART1 (`uart1_init`, `uart1_getc`, `uart1_putc`, ...) lo escribieron en el TP5. En este TP lo extienden con el **control de flujo** (hito 1) y lo usan como canal del procesador de comandos (hito 2). La referencia es [`datasheets/UART1_DATASHEET.md`](../datasheets/UART1_DATASHEET.md); para el hito 1 les interesa en especial la sección 5 (*Flow control*).

El procesador de comandos opera sobre UART1 **en los dos sentidos**: recibe las peticiones y manda las respuestas por ahí. UART0 queda para trazas de debug (`print_str`, etc.): no mezclen las trazas con el tráfico del protocolo.

### 1.2 Cómo se prueba

Los tests de este TP se corren desde la raíz del repositorio, igual que los del resto de los prácticos:

```bash
make test_flow_control     # hito 1: control de flujo
make test_main             # hito 2: procesador de comandos (fw/app/main.c)
make test_main VERBOSE=1   # igual, e imprime cada byte que pasa por UART1
make wave                  # abre wave.vcd (queda el del último test que corriste)
```

Cada comando compila el programa (con bootloader), lo simula con un *host* que maneja la UART1 según un patrón de test (`fw/tests/<test>.txt`, ver 1.3) y termina con `[ OK ]` o `[FAIL]` y el motivo. El código de salida es distinto de cero si el test falla. `make test` corre además todos los tests del repositorio: hasta que completen los hitos, los dos de este TP (`test_flow_control` y `test_main`) fallan (es lo esperado). Los del TP5 (`test_drivers` y `test_uart_serial`) tienen que estar pasando desde el paso 0.

Hace falta el entorno del TP1 (toolchain RISC-V en `/opt/riscv`, Verilator y los submódulos inicializados).

Qué hay en cada carpeta:

```
fw/
├── drivers/inc/uart1.h                 interfaz del driver de UART1 (la del TP5)
├── drivers/src/uart1.c                 driver de UART1 (TP5): acá completan uart1_set_rts()  → HITO 1
├── tests/
│   ├── test_flow_control.c .txt        hito 1: aplicación y patrón  → COMPLETAN flow_control()
│   ├── slow_consumer.h                 consumidor lento del hito 1 (dado)
│   ├── main.txt                        patrón del hito 2 (dado)
│   ├── test_uart_serial.c  .txt        TP5: prueba del driver de UART1 (paso 0)
│   └── test_drivers.c                  TP5: chequeo de la cátedra de los drivers (paso 0)
├── lib/inc/clock.h                     contador de ciclos del core: clock_cycles(), clock_elapsed() (dado)
├── app/
│   ├── main.c                          procesador de comandos del hito 2  → LO COMPLETAN
│   └── cmd_protocol_defs.h             constantes del protocolo (dado)
└── tools/cmdframe.py                   arma tramas con LENGTH y CRC para sus propios tests

datasheets/UART1_DATASHEET.md           referencia de la UART1 (control de flujo: sección 5)
datasheets/DATASHEET.md                 referencia de la plataforma (mapa de memoria, registros)
```

Opciones del `make test_...`: `UART1_SCRIPT=fw/tests/mio.txt` usa otro patrón (ruta relativa a la raíz del repositorio), `VERBOSE=1` muestra cada byte que pasa por UART1 y `MAX_CYCLES=n` cambia el límite de clocks de simulación.

Si un test falla y no ven por qué, en la **§4 (Cómo debuggear)** están las herramientas: el log de la simulación, `VERBOSE=1`, patrones propios con `cmdframe.py` y las ondas.

Si en el hito 2 agregan módulos propios, ponen los `.c` y `.h` en `fw/app/` y listan cada `.c` en `fw/sources.mk` (`SRC += app/mi_modulo.c`), igual que los drivers.

> **Variables globales.** Con el flujo de la plataforma (bootloader + app) el bootloader copia `.data` y limpia `.bss` antes de llamar a `main()`, así que pueden usar variables globales inicializadas. Con `STANDALONE=1` (sin bootloader) eso no ocurre.

### 1.3 Los patrones de test

Cada patrón es un archivo de texto que describe **qué manda el host por UART1 y qué espera recibir**. El testbench lo ejecuta contra su firmware y reporta cada paso.

```
# comentario                                (también //)
baud 32                  clocks por bit del host (tiene que coincidir con el BAUD_DIV del FW)
gap 0                    clocks de idle extra entre bytes
skid 2                   bytes que el host manda igual después de ver RTS en alto
timeout 200000           clocks máximos de espera de cada '<' y de cada espera de RTS
> A5 5A 04 00 05 00 55 47 C3 3C      envía esos bytes (hex); 'b*N' repite b N veces: 00*40
>! 55                    envía el byte con el bit de stop en 0 (framing error)
< A5 5A 03 00 85 00 xx C3 3C         espera exactamente esos bytes; xx = cualquiera, xx*N = N cualquiera
                                     'crc' = el byte debe ser el CRC-8 de OPCODE..último dato
wait 2000                idle de N clocks
drain 12000              espera N clocks y descarta lo que haya llegado
expect_no_overrun        falla si la UART1 del firmware perdió algún byte en algún momento
```

El test termina con `CHECK PASS (n/n)` o `CHECK FAIL` (y `RESULT: PASS|FAIL`). Si el firmware se cuelga (por ejemplo, por escribir `REG_MCU_RESET`) el testbench corta por límite de clocks y falla, no queda colgado. Para armar tramas con LENGTH y CRC correctos usen `python3 fw/tools/cmdframe.py` (ver su ayuda): por ejemplo `python3 fw/tools/cmdframe.py req 05 00 55` imprime la línea `>` de un LED SET.

---

## 2. Hito 1 — Control de flujo

**Objetivo.** Recibir una ráfaga **continua** de bytes sin perder ninguno, cuando el consumidor es más lento que la línea.

**El problema.** El FIFO RX tiene 16 bytes. Un byte llega cada `10 × BAUD_DIV` = 320 clocks. Si el firmware se demora en otra cosa más que `16 × 320` clocks, el FIFO se llena y los bytes siguientes se pierden. En un procesador de comandos eso pasa todo el tiempo: mientras ejecuta un comando, no lee la UART.

**La solución.** El control de flujo RTS (ver §5 del datasheet): el firmware baja `RTS` cuando el FIFO se está llenando y el host deja de mandar; lo sube cuando vuelve a tener lugar.

**Qué se les da.** `fw/tests/test_flow_control.c` ya está armado: lee los bytes, los procesa con `procesar_lento()` (en `slow_consumer.h`: acumula un CRC-8 y tarda unos 3 tiempos de byte) y responde `crc8 len_lo len_hi`. El host de la prueba manda `len_lo len_hi` y después `len` bytes seguidos, sin pausas.

**Qué hacen.**
1. Completar `uart1_set_rts()` en `fw/drivers/src/uart1.c`: su driver del TP5, que la dejó pendiente.
2. En `flow_control()` de `fw/tests/test_flow_control.c`, con `uart1_rx_level()` y `rts_on`:
   - **bajar RTS** (`uart1_set_rts(0)`) cuando el FIFO está cerca de llenarse;
   - **volver a subirlo** (`uart1_set_rts(1)`) cuando el FIFO se vació lo suficiente.

El test falla mientras falte cualquiera de las dos mitades: sin la que baja RTS se pierden bytes (overrun); sin la que lo sube, el host se queda esperando y el test falla con *"el DUT nunca levantó RTS"*. Pasa cuando están las dos.

**Para pensar.** Con qué nivel hay que bajar RTS para que no llegue a llenarse: después de bajarlo todavía pueden llegar hasta 3 bytes (el que el host tenía en curso y `skid = 2` más), y mientras `procesar_lento()` corre entran hasta 3 más. El FIFO tiene 16 bytes. Y por qué el nivel para volver a subirlo tiene que ser menor que el de bajarlo.

**`make test_flow_control` verifica** ráfagas de 16, 256 y 1024 bytes contra el CRC esperado, y que la UART1 no marcó ningún overrun en toda la prueba (`expect_no_overrun`). El host siempre respeta RTS: si nunca lo suben, el test falla con *"el DUT nunca levantó RTS"*.

---

## 3. Hito 2 — Procesador de comandos

### 3.1 Objetivo

Implementar un **Command Processor** (`fw/app/main.c`; los módulos propios se agregan en esa misma carpeta y se listan en `fw/sources.mk`) que:

1. Se sincronice al flujo de bytes entrante por UART1 y detecte el comienzo de un paquete válido.
2. Valide la trama completa (HEADER, longitud declarada, CRC y FOOTER) **antes de ejecutar nada**.
3. Ante una trama corrupta, la descarte entera, no la ejecute y **responda un error** (ver §3.5).
4. Decodifique el opcode y sus parámetros.
5. Ejecute la acción usando los drivers de ADC, DMA, memoria (bus) o LED ya desarrollados.
6. Devuelva por UART1 un paquete de respuesta con el resultado (éxito + datos, o falla + código de error).
7. Se recupere ante tramas corruptas o incompletas, sin quedar bloqueado ni requerir un reset.
8. **Use el control de flujo (RTS) del hito 1**: cuando el procesador no está leyendo la UART —verificando el CRC, ejecutando un comando o mandando la respuesta— tiene que frenar al host, o el FIFO RX se llena y se pierden bytes (ver §3.5).
9. **Valide que las direcciones de memoria de cada comando sean seguras antes de usarlas** (ver §3.7): en esta plataforma una dirección fuera de rango no genera una excepción, **cuelga el núcleo indefinidamente** (`datasheets/DATASHEET.md`, §2: *"never asserts `mem_ready`, so the CPU stalls forever"*). Dejar pasar una dirección inválida a un comando Load/Store o DMA tira abajo toda la simulación, incluida la respuesta de error que se supone deben mandar.

**El control de flujo es parte del requisito, no un extra:** el patrón del hito 2 manda tramas largas pegadas y falla si se pierde algún byte (ver §3.5 y §3.6).

### 3.2 Especificación del protocolo

#### Formato general del paquete (request)

```
 ┌──────────┬─────────┬────────┬──────────────┬─────┬──────────┐
 │  HEADER  │ LENGTH  │ OPCODE │   PAYLOAD    │ CRC │  FOOTER  │
 │ 2 bytes  │ 2 bytes │ 1 byte │  N bytes     │1byte│ 2 bytes  │
 └──────────┴─────────┴────────┴──────────────┴─────┴──────────┘
```

| Campo    | Tamaño | Valor / descripción |
|----------|--------|----------------------|
| HEADER   | 2 bytes | `0xA5 0x5A` — patrón fijo, poco probable que aparezca por azar en datos random. Marca el posible inicio de un paquete. |
| LENGTH   | 2 bytes, little-endian | Cantidad de bytes **desde OPCODE hasta CRC inclusive** (`1 (opcode) + tamaño del payload + 1 (crc)`). Se usa para saber dónde debería estar el FOOTER y confirmar que el paquete es consistente. Valores válidos: **de 2 a 1032** (1032 = `Memory Write` de 256 palabras: `1 + 6 + 1024 + 1`). |
| OPCODE   | 1 byte | Código de operación (ver tabla de comandos). |
| PAYLOAD  | Variable (según OPCODE) | Parámetros del comando. |
| CRC      | 1 byte | CRC-8 calculado sobre `OPCODE + PAYLOAD` (ver más abajo). |
| FOOTER   | 2 bytes | `0xC3 0x3C` — cola fija, con bytes distintos de los del header para que ninguno se confunda con el comienzo de un `HEADER`. Si después de leer `LENGTH` bytes no aparece exactamente este valor, el paquete es inválido (ver §3.5). |

#### CRC

CRC-8 con polinomio `0x07` (CRC-8/CCITT) y valor inicial `0xFF`, calculado sobre `OPCODE + PAYLOAD` (no incluye HEADER, LENGTH ni FOOTER). Es el mismo CRC que usa el bootloader para validar la imagen del FW.

**No lo reescriban:** la biblioteca de la plataforma ya lo tiene, `str_crc8()` en `fw/lib/src/strutils.c` (declarada en `fw/lib/inc/strutils.h`; sumen `lib/src/strutils.c` a `fw/sources.mk` con `SRC += lib/src/strutils.c`):

```c
#include "strutils.h"

uint8_t crc = str_crc8(body, body_len);     /* body = OPCODE + PAYLOAD */
```

Para comprobar a mano: el CRC de `{0x01, 0x02, 0x03, 0x04}` es `0x32`.

#### Endianness

Todos los campos multi-byte van en **little-endian**, consistente con el RV32I de la plataforma.

### 3.3 Comandos soportados

| Comando | OPCODE | Payload (request) |
|---|---|---|
| ADC Acquisition | `0x01` | `clk_divider` (4 B) + `mode` (1 B) + `num_samples` (2 B) |
| DMA Transfer | `0x02` | `src_addr` (4 B) + `dst_addr` (4 B) + `size_words` (4 B) |
| Memory Read | `0x03` | `addr` (4 B) + `length_words` (2 B) |
| Memory Write | `0x04` | `addr` (4 B) + `length_words` (2 B) + `data` (`length_words * 4` B) |
| LED Command | `0x05` | `op` (1 B) + `value` (1 B, solo si `op == SET`) |

**ADC Acquisition (`0x01`).** Dispara una adquisición en el ADC y devuelve las muestras capturadas. El ADC de esta plataforma no es un conversor por canal bajo demanda: siempre adquiere un bloque fijo de 256 muestras.
- `clk_divider` (4 bytes): frecuencia de muestreo a usar (`REG_ADC_CLK_DIVIDER`). Fíjenlo con `adc_init()` antes de adquirir. Ojo: `adc_read()` tiene que usar el divisor ya configurado; si configurara uno propio, el que llega en el comando se ignoraría.
- `mode` (1 byte): bit 0 = modo circular (continuo). **El modo single-shot (bit en 0) es el caso obligatorio de este TP**; el circular queda como opcional/bonus, a definir y documentar por el grupo si lo implementan.
- `num_samples` (2 bytes): cuántas de las 256 muestras adquiridas se devuelven (**1 a 256**).

**DMA Transfer (`0x02`).** Transfiere un bloque de memoria a memoria con el DMA de hardware.
- `src_addr`, `dst_addr` (4 bytes cada una): direcciones de byte, alineadas a palabra (múltiplo de 4).
- `size_words` (4 bytes): cantidad de **palabras de 32 bits** a transferir (`REG_DMA_SIZE` se mide en palabras, no en bytes). El DMA copia exactamente `size_words` palabras.

**Memory Read / Memory Write (`0x03` / `0x04`).** Leer o escribir memoria directamente desde la CPU, en **palabras de 32 bits**.
- `addr` (4 bytes): dirección de byte, **múltiplo de 4**.
- `length_words` (2 bytes): cantidad de palabras de 32 bits a leer o escribir, **de 1 a 256**.
- `data` (solo en `Memory Write`, `length_words * 4` bytes): las palabras a escribir, little-endian consecutivas.

**LED Command (`0x05`).** Fija o lee el patrón de 8 LEDs (`REG_LED`).
- `op` (1 byte): `0x00 = SET`, `0x01 = GET`.
- `value` (1 byte, **solo presente si `op == SET`**): nuevo patrón de 8 bits.

Si `op == GET`, el request no incluye `value` (ajustar `LENGTH` y el CRC en consecuencia).

### 3.4 Formato de la respuesta

Misma estructura que el request (HEADER, LENGTH, OPCODE, PAYLOAD, CRC, FOOTER).

| Campo | Valor |
|---|---|
| HEADER / FOOTER | Iguales al request |
| OPCODE | `OPCODE_RESP = OPCODE_REQUEST \| 0x80` |
| PAYLOAD | `STATUS` (1 byte) + datos asociados |
| CRC | CRC-8 sobre `OPCODE_RESP + PAYLOAD` |

**STATUS**

| Valor | Significado |
|---|---|
| `0x00` | OK |
| `0x01` | ERROR_CRC — el CRC recibido no coincide con el calculado sobre `OPCODE + PAYLOAD` |
| `0x02` | ERROR_LENGTH — el payload no tiene el tamaño que corresponde al opcode, el FOOTER no está donde dice `LENGTH`, o `LENGTH` está fuera de 2..1032 |
| `0x03` | ERROR_UNKNOWN_OPCODE |
| `0x04` | ERROR_INVALID_PARAM (incluye: dirección fuera de rango, dirección no alineada, acceso que tocaría `REG_MCU_RESET`, `length_words` o `num_samples` fuera de 1..256, `size_words` = 0 — ver §3.7) |
| `0x05` | ERROR_HW (falla del periférico durante la ejecución) |

Si `STATUS != 0x00`, el PAYLOAD de la respuesta termina ahí.

**Datos de respuesta por comando (cuando STATUS = OK)**

| Comando | Datos adicionales en el PAYLOAD de la respuesta |
|---|---|
| ADC Acquisition | `num_samples` muestras de 32 bits (little-endian), tal como fueron leídas de `REG_ARRAY_ADC_BUFF` |
| DMA Transfer | Sin datos adicionales |
| Memory Read | `length_words` palabras de 32 bits leídas |
| Memory Write | Sin datos adicionales |
| LED Command (`GET`) | 1 byte: valor actual de `REG_LED` (bits bajos) |
| LED Command (`SET`) | Sin datos adicionales |

Ejemplo — `LED SET 0x55` y su respuesta (con `fw/tools/cmdframe.py`):

```
> A5 5A 04 00 05 00 55 47 C3 3C
< A5 5A 03 00 85 00 20 C3 3C
```

### 3.5 Requisitos funcionales

El procesador de comandos debe:

1. **Sincronizarse al stream**: identificar el comienzo de un paquete válido dentro de un flujo continuo de bytes, ignorando la basura que haya antes de un `HEADER`.
2. **Validar la trama completa antes de ejecutar nada**: `HEADER`, `LENGTH` (de 2 a 1032), CRC y `FOOTER`. El comando se ejecuta **recién cuando la trama entera llegó y es correcta**.
3. **Tramas corruptas**: si un byte llega corrupto, la trama se **descarta entera, no se ejecuta, y se responde un error**:

   | Falla | Respuesta |
   |---|---|
   | CRC incorrecto | `ERROR_CRC`, con el `OPCODE` de la petición |
   | `FOOTER` distinto de `0xC3 0x3C` | `ERROR_LENGTH`, con el `OPCODE` de la petición |
   | `LENGTH` fuera de 2..1032 | `ERROR_LENGTH` con `OPCODE` 0: la respuesta lleva `OPCODE_RESP = 0x80`, porque todavía no se recibió el `OPCODE` |
   | Aparece otro `HEADER` (`0xA5 0x5A`) antes de completar la trama | `ERROR_LENGTH`, con el `OPCODE` de la trama interrumpida (0 si todavía no se había recibido); la trama nueva se procesa normalmente |

   Si fallan el CRC y el `FOOTER` a la vez, se responde `ERROR_CRC`. Una trama con `LENGTH` válido se consume **hasta su `FOOTER` o hasta que aparezca otro `HEADER`**. Si ya se sabe que la trama está mal igual se la consume hasta su `FOOTER`: así el receptor queda alineado con el final de la trama.

   **No hay que preocuparse por que aparezca un `HEADER` dentro de un payload.** El protocolo no tiene mecanismo de escape: los bytes `0xA5 0x5A` seguidos, dentro de una trama, se toman por el comienzo de otra. Por eso **ningún comando del patrón lleva esa secuencia** en su `OPCODE`, `PAYLOAD` o `CRC`, y no hace falta contemplar ni proteger ese caso. (Un `0xA5` o un `0x5A` sueltos sí pueden aparecer en un payload, y no son un `HEADER`.)

   Lo que **no** es una trama corrupta y se descarta **sin responder**: bytes sueltos antes de un `HEADER`, un `0xA5` seguido de un byte que no es `0x5A` (indistinguible del ruido), y una trama que no se completa porque dejaron de llegar bytes (ver §3.6).
4. **Decodificar OPCODE y payload**, validando parámetros.
5. **Ejecutar la acción** usando los drivers ya desarrollados, respetando las secuencias de hardware (flancos de start, orden de escritura de registros, espera de "done"/"status").
6. **Responder por UART1** a cada trama completa, correcta o corrupta.
7. **Control de flujo (RTS).** Mientras el procesador no lee la UART, el host sigue mandando y el FIFO RX (16 bytes) se llena; los bytes que no entran **se pierden**. Hay que frenar al host con RTS (`uart1_set_rts(0)`) **antes** de cada tramo en el que no se va a leer la UART —verificar el CRC de una trama larga, ejecutar un comando, armar y mandar la respuesta— y volver a habilitarlo cuando se retoma la lectura. Después de bajar RTS todavía pueden llegar hasta 3 bytes (el que el host tenía en curso más `skid = 2`), así que hay que bajarlo con margen. El patrón del hito 2 manda dos `Memory Write` de 256 palabras pegados (ver §3.6): sin control de flujo el test falla por overrun.
8. **Recuperarse de errores sin bloquearse.**
9. **Validar rangos y alineación de direcciones antes de tocar el bus** (ver §3.7): esta plataforma no tiene manejo de excepciones por dirección inválida, así que la validación la hace el firmware, no el hardware.

### 3.6 Recuperación: lo que verifica el testbench

Para que el test sea determinista, el patrón del hito 2 (`fw/tests/main.txt`) asume lo siguiente:

- **Pausas dentro de una trama:** una pausa de hasta 2000 clocks entre dos bytes de una misma trama no la rompe: la trama se procesa normalmente.
- **Trama que no se completa:** si dejan de llegar bytes con una trama incompleta (por ejemplo, un `LENGTH` mayor que el real, o una trama truncada), el firmware tiene que descartarla, **sin responder** y sin ejecutarla, y volver a esperar un `HEADER` **antes de 150.000 clocks de silencio**. Después de ese tiempo tiene que procesar una trama válida normalmente. (Para medir ese tiempo, ver *Cómo medir el tiempo* más abajo.)
- **Tramas corruptas:** CRC incorrecto, `FOOTER` incorrecto o `LENGTH` fuera de rango reciben el error de §3.5 y **no se ejecutan** (el test comprueba que el estado del LED no cambió). A continuación una trama válida se procesa normalmente.
- **Trama interrumpida por otro `HEADER`:** una trama incompleta seguida **sin pausa** por otra válida: la primera recibe `ERROR_LENGTH` y la segunda se procesa normalmente. El patrón prueba tres casos: interrumpida justo después del `OPCODE`, después de varios bytes de cuerpo, y antes de recibir el `OPCODE` (la respuesta lleva `OPCODE` 0).
- **Bytes de protocolo sueltos en un payload:** un `Memory Write` cuyo payload tiene `0xA5` y `0x5A` sueltos (pero nunca `A5 5A` seguidos) se procesa normalmente.
- **Ruido:** bytes sueltos antes del `HEADER`, o un `0xA5` seguido de un byte que no es `0x5A`, se ignoran sin responder.
- **Tramas largas pegadas (control de flujo):** el host manda dos `Memory Write` de 256 palabras (1038 bytes cada uno) **sin esperar la respuesta de la primera**. Mientras el firmware verifica el CRC y ejecuta no está leyendo la UART: si no frena al host con RTS (`uart1_set_rts(0)`) antes de ese tramo, el FIFO RX se llena, se pierden bytes y el test falla (`expect_no_overrun`). Las dos respuestas tienen que ser correctas y el contenido de memoria el de la segunda escritura.
- **No se evalúan** otras situaciones de resincronización más allá de las de arriba: los casos del patrón son esos.

#### Cómo medir el tiempo

Descartar una trama por silencio obliga a medir cuánto pasó sin recibir bytes. La plataforma **no tiene un timer que se programe**, pero el core trae un **contador de ciclos**: sube de a uno por cada clock de la simulación. Son las mismas unidades que usa el patrón (`wait 150000`, `timeout 600000`, "una pausa de hasta 2000 clocks"). Como referencia, un byte de la UART1 tarda `10 × BAUD_DIV` = 320 clocks.

La cátedra les da el archivo `fw/lib/inc/clock.h` (ya está en el repositorio y el Makefile lo encuentra: alcanza con `#include "clock.h"`):

| Función | Qué hace |
|---|---|
| `clock_cycles()` | Devuelve el contador de ciclos (32 bits). |
| `clock_elapsed(t0)` | Clocks que pasaron desde `t0`, que es un valor que devolvió `clock_cycles()`. |

**Receta**, en tres pasos:

1. **Cada vez que llega un byte, anoten la hora**: `last_rx = clock_cycles();`
2. **Cuando no hay bytes para leer y hay una trama a medias**, pregunten cuánto pasó desde el último: `if (clock_elapsed(last_rx) > SILENCE_CLOCKS)`. Si es así, descarten la trama **sin responder** y vuelvan a esperar un `HEADER`.
3. **Elijan `SILENCE_CLOCKS`** entre dos límites: mayor que **2000** (una pausa normal dentro de una trama no la tiene que romper) y menor que **150000** (el patrón espera que ya la hayan descartado). Un valor en el medio, como `40000`, deja margen hacia los dos lados.

```c
#include "clock.h"

#define SILENCE_CLOCKS  40000u

static uint32_t last_rx;                 /* hora del ultimo byte recibido */

/* ... en el lazo de recepcion ... */
if (uart1_getc(&b)) {
    last_rx = clock_cycles();            /* paso 1: llego un byte */
    /* ... procesarlo ... */
} else if (trama_a_medias && clock_elapsed(last_rx) > SILENCE_CLOCKS) {
    trama_a_medias = 0;                  /* paso 2: demasiado silencio, se descarta sin responder */
}
```

Tres cuidados:

- **Anoten la hora cuando llega un byte, no cuando empiezan a esperar.** Si no, el tiempo que el firmware gasta ejecutando un comando se cuenta como silencio del host.
- **Miren el silencio solo si hay una trama a medias.** Entre tramas, esperando un `HEADER`, se puede esperar para siempre.
- **No cuenten vueltas del lazo como si fueran clocks.** Cuántas vueltas entran en 40000 clocks depende de qué tan rápido sea su código y cambia cada vez que lo tocan.

### 3.7 Validaciones de dirección obligatorias

Para los comandos `Memory Read`, `Memory Write` y `DMA Transfer`, el procesador de comandos **debe**, antes de ejecutar cualquier acceso al bus.

**Dónde ver cuáles son las direcciones válidas.** No las copien de esta consigna: el mapa de memoria está definido en un solo lugar, `project_config.def` (`ADDR_START_INST` / `ADDR_SIZE_INST`, `ADDR_START_DATA` / `ADDR_SIZE_DATA` y `ADDR_START_REG_FILE` / `ADDR_SIZE_REG_FILE`). De ahí se genera `fw/config.h`, que su código ya incluye a través de `includes.h`, así que en C pueden usar esas mismas macros. El mapa completo, con una tabla de regiones, está en `datasheets/DATASHEET.md`, §2 (*Global memory map*), y la descripción de cada registro en §4. Ojo: `REG_MCU_RESET` y el resto de los `REG_*` de `config.h` son macros que desreferencian el registro; para obtener su dirección usen `(uint32_t)&REG_MCU_RESET`. La lista de abajo es un resumen de esas fuentes.

El procesador de comandos **debe**:

- Verificar que la(s) dirección(es) y el rango completo accedido (`addr` hasta `addr + length_words*4 - 1`) caigan dentro de una región válida del mapa de memoria:
  - Memoria de instrucciones: `0x0000_0000`–`0x0001_FFFF`
  - Memoria de datos: `0x0002_0000`–`0x0003_FFFF`
  - Register file: `0x0004_0000`–`0x0005_FFFF`
  - Cualquier acceso fuera de `0x0000_0000`–`0x0005_FFFF`, o que cruce de una región a otra, debe rechazarse con `ERROR_INVALID_PARAM` **sin llegar a emitir la transacción de bus** (una dirección inválida no da error de hardware: **cuelga el core para siempre**).
- Verificar alineación a palabra (`addr % 4 == 0`) para estos tres comandos.
- **Rechazar explícitamente cualquier acceso de `Memory Write` cuyo rango incluya `REG_MCU_RESET` (`0x4_0014`)**: escribir `1` ahí detiene el núcleo y, según la datasheet, *"since the CPU can't execute while held in reset, it cannot clear this register itself"* — el propio firmware no tiene forma de revertirlo. Dejar pasar esta escritura cuelga la simulación entera sin posibilidad de recuperación por software. Esto se testea explícitamente.
- Direcciones del register file que no correspondan a ningún registro definido no están garantizadas a leer `0` (la datasheet aclara que son latches inferidos sin default), así que no asuman ese valor en sus tests.

**Cuidado con los efectos secundarios del register file.** No se les pide bloquear *todos* los posibles efectos colaterales de acceder al register file por `Memory Read`/`Write`/`DMA`; eso queda a su criterio, y pueden restringir más el rango permitido documentando la decisión. Dos casos para tener en cuenta: re-disparar un DMA escribiendo `REG_DMA_START`, y —nuevo con esta UART— **leer `REG_UART1_RX_DATA` (`0x4_3010`) saca un byte del FIFO**, es decir, un `Memory Read` o una lectura del DMA sobre esa dirección se comería bytes de su propio protocolo. Lo único que es requisito duro y evaluado es la protección de `REG_MCU_RESET` y las validaciones de rango y alineación de arriba.

---

## 4. Cómo debuggear

Cuando un test falla tienen cuatro herramientas, de la más simple a la más detallada: leer el motivo del fallo, ver cada byte con `VERBOSE=1`, escribir sus propios patrones de test con `cmdframe.py`, y mirar las ondas. Todo se corre desde la raíz del repositorio.

### 4.1 Leer el motivo del fallo

Un test que falla termina así en la consola:

```
[FAIL] test_main     esperado=PASS obtenido=FAIL (ver fw/build/test_main.sim.log)
         [FAIL] respuesta distinta. esperado: a5 5a 04 00 85 00 56 4c c3 3c  recibido: a5 5a 04 00 85 00 55 4c c3 3c
         CHECK FAIL (1 fallas de 3)
```

La consola muestra solo las primeras líneas del problema. **Todo lo que pasó está en `fw/build/<test>.sim.log`** (por ejemplo `test_main.sim.log`). Tiene colores; para leerlo cómodo:

```bash
sed 's/\x1b\[[0-9;]*m//g' fw/build/test_main.sim.log | less
```

Los mensajes más comunes y qué mirar:

| Mensaje | Qué significa |
|---|---|
| `respuesta distinta. esperado: ... recibido: ...` | El firmware respondió, pero no lo que el patrón espera. Comparen byte a byte: el `STATUS`, el `LENGTH`, el CRC. |
| `timeout esperando N bytes (llegaron M)` | El firmware respondió de menos o no respondió. ¿Se colgó? ¿Descartó la trama? ¿Esperaba más bytes de los que llegaron? |
| `N bytes recibidos que el script no esperaba` | El firmware mandó bytes de más (una respuesta que no tocaba, o un byte repetido). |
| `el DUT nunca levanto RTS (esta listo para recibir?)` | El host espera RTS antes de cada byte y el firmware lo dejó en 0 (no listo) para siempre: falta subirlo (`uart1_set_rts(1)`) al arrancar o después de procesar. |
| `la UART1 del DUT perdio bytes (overrun)` | Llegaron más bytes de los que el FIFO podía guardar: falta frenar al host con RTS antes de un tramo en que no se lee la UART (§2). |
| `el core hizo TRAP` | El procesador ejecutó una instrucción inválida o hizo un acceso desalineado. |
| `timeout: el script no termino en N clocks (FW colgado?)` | El firmware se colgó o está en un lazo que no termina. Ver 4.5. |
| `token invalido ...` / `comando desconocido ...` | Error de sintaxis en **su** archivo `.txt` (ojo: `xx` solo se puede usar en las líneas `<`). |

### 4.2 `VERBOSE=1`: ver cada byte que pasa por UART1

```bash
make test_main VERBOSE=1
sed 's/\x1b\[[0-9;]*m//g' fw/build/test_main.sim.log | grep -a "UART1"
```

Imprime un renglón por cada byte, en el orden en que pasan por la línea: `->` es lo que **manda el host al firmware** y `<-` lo que **responde el firmware**. Para un `LED SET 0x55` se ve así:

```
UART1 -> 0xa5            host manda el HEADER...
UART1 -> 0x5a
UART1 -> 0x04            ...LENGTH...
UART1 -> 0x00
UART1 -> 0x05            ...OPCODE (LED), op SET, value 0x55...
UART1 -> 0x00
UART1 -> 0x55
UART1 -> 0x47            ...CRC y FOOTER
UART1 -> 0xc3
UART1 -> 0x3c
UART1 <- 0xa5            el firmware responde: HEADER, LENGTH = 3...
UART1 <- 0x5a
UART1 <- 0x03
UART1 <- 0x00
UART1 <- 0x85            ...OPCODE_RESP = 0x05 | 0x80, STATUS = 0x00 (OK), CRC y FOOTER
UART1 <- 0x00
UART1 <- 0x20
UART1 <- 0xc3
UART1 <- 0x3c
[ OK ] recibido: a5 5a 03 00 85 00 20 c3 3c
```

(Los comentarios a la derecha no los imprime el testbench: están para explicar cada byte.) Con eso ven si el firmware respondió, qué respondió y en qué punto de la trama dejó de hacerlo. `VERBOSE=1` funciona con cualquier test (`test_uart_serial`, `test_flow_control`, `test_main`).

**Trazas propias por UART0.** En el firmware pueden usar `print_str()` / `print_dec()`; salen por UART0 y aparecen en el mismo log como líneas `UART: ...`, mezcladas en orden con los bytes de UART1. No usen UART1 para trazas: es el canal del protocolo. Tengan en cuenta que imprimir es bloqueante: mientras el firmware imprime no lee la UART1, así que no conviene imprimir dentro del lazo de recepción con tramas largas (ver control de flujo, §2).

### 4.3 Escribir sus propios patrones de test

Para aislar un problema conviene un archivo chico con solo el caso que falla. Pueden crear cualquier archivo `.txt` (por ejemplo `fw/tests/mio.txt`) y correrlo con `UART1_SCRIPT`, con la ruta **relativa a la raíz del repositorio**:

```bash
make test_main UART1_SCRIPT=fw/tests/mio.txt
make test_main UART1_SCRIPT=fw/tests/mio.txt VERBOSE=1     # y además ver cada byte
```

Un patrón es una configuración seguida de pares "lo que manda el host" (`>`) y "lo que espera recibir" (`<`) (el formato completo está en 1.3). Este es uno completo y funcional, que prende los LEDs y los vuelve a leer:

```
# mi primer caso: prender los LEDs y leerlos
baud 32
gap 0
skid 2
timeout 600000

# LED SET 0x55 -> OK
> A5 5A 04 00 05 00 55 47 C3 3C
< A5 5A 03 00 85 00 20 C3 3C

# LED GET -> OK con el dato 0x55
> A5 5A 03 00 05 01 91 C3 3C
< A5 5A 04 00 85 00 55 4C C3 3C

expect_no_overrun
```

Las primeras líneas son la configuración que usan los patrones de la cátedra: `baud 32` (tiene que coincidir con el `BAUD_DIV` del firmware) y `skid 2`. El host **siempre respeta RTS**: antes de cada byte mira la línea y, si el firmware la dejó en 0 (no listo), espera a que la suba. Cada `<` espera **exactamente** esos bytes: si el firmware responde otra cosa, el test muestra qué esperaba y qué recibió:

```
[FAIL] test_main     esperado=PASS obtenido=FAIL (ver fw/build/test_main.sim.log)
         [FAIL] respuesta distinta. esperado: a5 5a 04 00 85 00 56 4c c3 3c  recibido: a5 5a 04 00 85 00 55 4c c3 3c
         CHECK FAIL (1 fallas de 3)
```

Tip: `fw/tests/main.txt` es el patrón de la cátedra y sirve de modelo. Tiene un caso para cada situación (comandos correctos, errores de parámetros, direcciones inválidas, tramas corruptas, trama incompleta, etc.) con un comentario que explica qué prueba. Copien el caso que les interesa a un archivo propio y modifíquenlo.

### 4.4 Armar las tramas con `cmdframe.py`

Calcular a mano el `LENGTH`, el CRC y el footer de cada trama es tedioso y propenso a errores. `fw/tools/cmdframe.py` lo hace por ustedes: le pasan el `OPCODE` y el `PAYLOAD` en hexadecimal y devuelve la línea lista para pegar en el patrón (con `HEADER`, `LENGTH`, CRC y `FOOTER`). Se corre desde la raíz del repositorio:

```bash
python3 fw/tools/cmdframe.py req  <OPCODE> <PAYLOAD...>     # una línea '>' (lo que manda el host)
python3 fw/tools/cmdframe.py resp <OPCODE_RESP> <STATUS> <datos...>   # una línea '<' (lo que se espera recibir)
```

Los campos de varios bytes van en **little-endian**, como en el protocolo. Ejemplos:

| Qué quieren probar | Comando | Línea que imprime |
|---|---|---|
| LED SET `0x55` | `cmdframe.py req 05 00 55` | `> A5 5A 04 00 05 00 55 47 C3 3C` |
| …su respuesta OK | `cmdframe.py resp 85 00` | `< A5 5A 03 00 85 00 20 C3 3C` |
| LED GET | `cmdframe.py req 05 01` | `> A5 5A 03 00 05 01 91 C3 3C` |
| …respuesta con el dato `0x55` | `cmdframe.py resp 85 00 55` | `< A5 5A 04 00 85 00 55 4C C3 3C` |
| Memory Write de 1 palabra `0xDEADBEEF` en `0x0002_0000` | `cmdframe.py req 04 00 00 02 00 01 00 EF BE AD DE` | `> A5 5A 0C 00 04 00 00 02 00 01 00 EF BE AD DE 25 C3 3C` |
| …su respuesta OK | `cmdframe.py resp 84 00` | `< A5 5A 03 00 84 00 35 C3 3C` |
| Memory Read de 1 palabra en `0x0002_0000` | `cmdframe.py req 03 00 00 02 00 01 00` | `> A5 5A 08 00 03 00 00 02 00 01 00 1B C3 3C` |
| …respuesta con el dato leído | `cmdframe.py resp 83 00 EF BE AD DE` | `< A5 5A 07 00 83 00 EF BE AD DE AD C3 3C` |
| Opcode desconocido `0x7F` | `cmdframe.py req 7F` | `> A5 5A 02 00 7F 89 C3 3C` |
| …`ERROR_UNKNOWN_OPCODE` (`0x03`) | `cmdframe.py resp FF 03` | `< A5 5A 03 00 FF 03 09 C3 3C` |
| Memory Read fuera del mapa (`0x0006_0000`) | `cmdframe.py req 03 00 00 06 00 01 00` | `> A5 5A 08 00 03 00 00 06 00 01 00 43 C3 3C` |
| …`ERROR_INVALID_PARAM` (`0x04`) | `cmdframe.py resp 83 04` | `< A5 5A 03 00 83 04 42 C3 3C` |

(En la columna "Comando" falta el prefijo `python3 fw/tools/` para que entre en la tabla.) Para los datos de los campos ver §3.3, y para el `STATUS` de la respuesta §3.4: el `OPCODE` de la respuesta es el del request con el bit `0x80` encendido (`05` → `85`).

**Tramas con errores a propósito**, para probar la validación:

| Opción | Efecto | Ejemplo |
|---|---|---|
| `--bad-crc` | CRC incorrecto | `cmdframe.py req 05 00 55 --bad-crc` → `> A5 5A 04 00 05 00 55 1D C3 3C` (espera `ERROR_CRC`: `cmdframe.py resp 85 01`) |
| `--bad-footer` | `FOOTER` incorrecto | `cmdframe.py req 05 00 55 --bad-footer` → `> A5 5A 04 00 05 00 55 47 C3 C3` |
| `--len-delta N` | `LENGTH` declarado `N` bytes mayor (o menor, si es negativo) que el real | `cmdframe.py req 05 00 55 --len-delta 3` → `> A5 5A 07 00 05 00 55 47 C3 3C` |

Una trama con `--len-delta` positivo queda **incompleta**: el firmware espera bytes que no llegan. Para probar que se recupera, agreguen una pausa y una trama válida (así lo hace `main.txt`):

```
> A5 5A 07 00 05 00 55 47 C3 3C          # LENGTH dice 7, la trama tiene menos
wait 150000                                # silencio: el firmware tiene que descartarla sin responder
> A5 5A 03 00 05 01 91 C3 3C              # LED GET, tiene que responder normalmente
< A5 5A 04 00 85 00 55 4C C3 3C
```

**Respuestas con datos variables.** Cuando no pueden calcular un dato a mano (por ejemplo las muestras del ADC), usen `xx` en vez del byte (`xx*64` = 64 bytes cualquiera) y `crc` en vez del CRC, que lo calcula el testbench. Con `--crc-token`, `cmdframe.py` escribe el `crc` por ustedes:

```
< A5 5A 43 00 81 00 xx*64 crc C3 3C      # respuesta de un ADC de 16 muestras
```

**Armar un archivo entero de una pasada.** Como las líneas salen por la salida estándar, se puede redirigir. Este comando genera el patrón de la sección 4.3 completo:

```bash
C="python3 fw/tools/cmdframe.py"
{
  echo "baud 32"; echo "gap 0"; echo "skid 2"; echo "timeout 600000"
  $C req 05 00 55;  $C resp 85 00        # LED SET 0x55 y su respuesta
  $C req 05 01;     $C resp 85 00 55     # LED GET y su respuesta
  echo "expect_no_overrun"
} > fw/tests/mio.txt
```

**Hito 1.** `cmdframe.py stream --len N --seed S` arma las líneas de una ráfaga de `N` bytes con datos pseudoaleatorios y el CRC-8 esperado de la respuesta; es lo que usa `test_flow_control.txt`. Por ejemplo `cmdframe.py stream --len 4 --seed 1` imprime:

```
> 04 00
> 44 20 82 3C
< 79 04 00
```

### 4.5 Ondas y firmware colgado

Cada `make test_<nombre>` individual deja `wave.vcd` (`make test`, que corre todos los tests, no lo guarda). Se abre con `make wave`. Las señales de UART1 que más sirven:

| Señal | Qué es |
|---|---|
| `u_top.uart1_rx_line` | línea que el host maneja hacia el firmware (RX del firmware) |
| `u_top.uart1_tx_line` | línea con la respuesta del firmware (TX del firmware) |
| `u_top.uart1_rts_n` | RTS del firmware (**activo en bajo**: `0` = listo para recibir) |

Con `uart1_rts_n` ven exactamente cuándo el firmware frena y cuándo habilita al host, y con las dos líneas de datos cada bit de cada byte (`BAUD_DIV` = 32 clocks por bit).

Si el test termina con `el core hizo TRAP` o `timeout: el script no termino en N clocks (FW colgado?)`, casi siempre es una dirección inválida que llegó al bus (§3.7: una dirección fuera del mapa cuelga el core) o un lazo del firmware que no termina. Abran las ondas y busquen la última actividad. Los tests con patrón cortan a los 12.000.000 de clocks; si su firmware es lento y necesitan más tiempo, `make test_main MAX_CYCLES=30000000`.

---

## 5. Lineamientos de diseño

- El diseño interno (driver, sincronización de trama, decodificación, despacho de comandos) queda a criterio de cada grupo, con una condición: **tienen que aplicar al menos un patrón de diseño de los vistos en las clases teóricas** en alguna parte del procesador de comandos.
- Indiquen en la entrega **qué patrón eligieron, en qué parte del código lo aplicaron y qué problema les resuelve**.
- **En la evaluación se pone el énfasis en este punto: qué patrones de diseño implementaron y, sobre todo, por qué tomaron esa decisión.** Expliquen qué problema concreto del procesador de comandos resuelve cada patrón, qué alternativas consideraron y descartaron, y qué costo tiene su elección. Un patrón aplicado sin justificación, o que no resuelve un problema real del código, no suma.
- Pueden refactorizar el código de drivers de prácticos anteriores si el diseño elegido lo requiere.
- No hay restricciones de tiempo real ni de memoria, pero eviten espera activa que bloquee indefinidamente la recepción de nuevos bytes.
- Se valora que agregar un comando nuevo no obligue a reescribir la lógica de sincronización y validación de tramas.

---

## 6. Entrega y evaluación

- **Qué se entrega**: código fuente únicamente (`fw/drivers/src/uart1.c`, `fw/tests/test_flow_control.c`, `fw/app/main.c`, los módulos propios y `fw/sources.mk` si agregaron alguno), con `make test_flow_control` y `make test_main` pasando (y los tests del TP5, `make test_drivers` y `make test_uart_serial`, que son el paso 0).
- **Tests propios**: agreguen sus propios patrones en `fw/tests/` (casos de error, resincronización, los límites de §3.7, etc.), ejecútenlos con `make test_main UART1_SCRIPT=fw/tests/mio.txt` (desde la raíz del repositorio), y mencionen en la entrega qué cubre cada uno.
- **Entorno de evaluación**: simulación con Verilator sobre `rtl/testbench.sv` (con el host de UART1 de `rtl/uart1_host.svh`) y `rtl/top.v`.

### Criterios de evaluación (orientativos)

- Correctitud: los dos tests de este TP (`test_flow_control` y `test_main`) pasan, y los del TP5 (paso 0) siguen pasando.
- Control de flujo (hito 1): uso correcto de RTS según el datasheet (bajarlo con margen, subirlo con histéresis).
- Robustez (hitos 1 y 2): ninguna pérdida de bytes bajo ráfaga continua, incluidas las tramas largas que llegan pegadas (control de flujo); recuperación ante tramas corruptas, incompletas o ruidosas sin bloquearse.
- **Seguridad de acceso a memoria**: validación de rangos, alineación y protección de `REG_MCU_RESET` (§3.7) — se testea puntualmente.
- Calidad de diseño (es donde se pone más énfasis): qué patrones de diseño implementaron y la justificación de por qué los eligieron (problema que resuelven, alternativas descartadas); separación entre sincronización/decodificación de trama y ejecución de comandos; facilidad para agregar un comando nuevo.
- Cobertura y pertinencia de los tests propios.
- Legibilidad y organización general del código.
