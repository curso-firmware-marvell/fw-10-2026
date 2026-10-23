# Proyecto de prueba Binutils — orientado a embebidos

Proyecto práctico para explorar **GNU Binutils** mediante código en **C puro** con patrones típicos del **desarrollo embebido**: aritmética de punto fijo, solo memoria estática, manipulación de bits, registros de hardware simulados y buffers circulares.

## Estructura del proyecto

```
actividades_de_clase/
├── Makefile                # Sistema de compilación (gcc + g++, C11/C++17, flags embebidos)
├── README.md               # Este archivo
├── test_binutils.sh        # Script principal (ejercita las 13 herramientas)
└── src/
    ├── app/                # Punto de entrada de la aplicación
    │   └── main.c          # main reducido — llama a cada sección demo
    ├── lib/                # Bibliotecas reutilizables e independientes de plataforma
    │   ├── mathlib.h/.c    # Matemática entera, punto fijo Q16.16, bits, LUT seno, matrices
    │   └── strutils.h/.c   # Cadenas en buffers de tamaño fijo, tokenizer, CRC-8
    ├── hal/                # Módulos de capa de abstracción de hardware (HAL)
    │   ├── hal_uart.h/.c   # Estructura de registros UART, init, envío
    │   ├── hal_ring.h/.c   # Buffer circular estático (sin heap)
    │   └── hal_system.h/.c # Función weak BSP, ISR SysTick, defs GPIO, externs ASM
    ├── demo/               # Código de demostración / harness de prueba
    │   ├── demo.h          # Declaraciones comunes de las demos
    │   ├── demo_math.c     # Demos de math, punto fijo, LUT, bits, matrices
    │   ├── demo_str.c      # Demos de utilidades de cadenas, tokenizer, CRC-8
    │   └── demo_embedded.c # Demos UART, ring buffer, ISR, ASM, inline-ASM
    ├── cpp/                # Capa envoltorio C++ (para c++filt / símbolos mangled)
    │   ├── cpp_wrapper.h   # Namespaces, clases, templates, vtable sobre libs C
    │   └── cpp_wrapper.cpp # Implementación + punto de entrada run_cpp_demo()
    └── asm/                # Fuentes en ensamblador
        └── asm_example.s   # Ensamblador x86-64 (para probar `as`)
```

## Principios de diseño embebido

- **Cero asignación en heap** — sin `malloc`, `new` ni contenedores STL
- **C puro (C11)** — sin características C++ en el núcleo; nombres de símbolos predecibles
- **Buffers de tamaño fijo** — operaciones de cadenas con arrays en stack o estáticos y límites explícitos
- **Matemática de punto fijo** — formato Q16.16 sin hardware de coma flotante
- **Tablas de consulta (LUT)** — seno/coseno precalculados (común en DSP/control de motores)
- **Manipulación de bits** — set/clear/toggle/test/count/reverse (operaciones a nivel de registro)
- **Registros volátiles** — periférico UART simulado con struct memory-mapped
- **Buffer circular** — buffer estático para transferencia ISR → main
- **Símbolos débiles (weak)** — `board_early_init()` reemplazable vía `__attribute__((weak))`
- **Patrón ISR** — `SysTick_Handler()` modifica flags volátiles
- **CRC-8** — checksum usado en SMBus, 1-Wire y protocolos embebidos
- **Flags del compilador** — `-ffunction-sections -fdata-sections -Wl,--gc-sections` (eliminación de código muerto, estándar en toolchains embebidos)

### Capa envoltorio C++ (`cpp_wrapper.h/.cpp`)

Muestra el patrón embebido habitual de envolver drivers C con una API C++:

- **Namespace** — `hal::` agrupa las abstracciones C++
- **Clases sobre structs C** — `FixedPoint`, `Matrix2x2`, `StringBuffer` delegan en funciones C
- **Sobrecarga de operadores** — `+`, `-`, `*`, `/`, `==`, `<` en `FixedPoint`
- **Ring buffer con template** — `RingBuffer<T, N>` con capacidad en tiempo de compilación (sin heap)
- **Despacho virtual** — interfaz `Peripheral` con implementación `UartPeripheral`
- **Namespaces anidados** — `hal::trig::sin()`, `hal::trig::cos()`
- **Puente `extern "C"`** — `run_cpp_demo()` invocable desde `main.c`

Genera **símbolos C++ mangled**, necesarios para demostrar `c++filt`, `nm --demangle` y contrastar tablas de símbolos C vs C++.

## Artefactos de compilación

| Artefacto | Flags | Propósito |
|-----------|-------|-----------|
| `build/binutils_demo` | `-g -O0` + `--gc-sections` | Compilación por defecto, enlazada con los objetos sueltos |
| `build/binutils_demo_nogc` | `-g -O0` | Mismos objetos sin `--gc-sections`, para comparar con `size` |
| `build/binutils_demo_lib` | `-g -O0` + `-lutils` | `app/` y `demo/` enlazados contra `libutils.a` |
| `build/binutils_demo_debug` | `-g3 -O0` | Info de debug completa para `addr2line`, `objdump -S` |
| `build/binutils_demo_opt` | `-O2 -gc-sections` | Optimizado con eliminación de código muerto |
| `build/binutils_demo_stripped` | `-O2` + `strip` | Sin símbolos — muestra qué queda tras strip |
| `build/libutils.a` | | Archivo estático de mathlib + strutils |
| `build/*.o` | | Archivos objeto individuales |

## Inicio rápido

```bash
make all                      # Compilar todo
./build/binutils_demo         # Ejecutar el programa demo
bash test_binutils.sh         # Ejecutar las 13 pruebas de binutils
bash test_binutils.sh nm      # Solo la sección de una herramienta
bash test_binutils.sh --help  # Listar herramientas disponibles
```

## Herramientas ejercitadas

| # | Herramienta | Propósito |
|---|-------------|-----------|
| 1 | `as` | Ensamblador GNU — traduce `.s` a objetos |
| 2 | `ld` | Enlazador GNU — combina objetos en ejecutables |
| 3 | `nm` | Lista símbolos (funciones/variables) en objetos |
| 4 | `objdump` | Muestra cabeceras, desensamblado, reubicaciones |
| 5 | `readelf` | Analiza estructura ELF (cabeceras, segmentos, secciones) |
| 6 | `ar` | Gestiona archivos de biblioteca estática (`.a`) |
| 7 | `ranlib` | Genera índice para archivos `.a` |
| 8 | `strip` | Elimina símbolos e info de debug del binario |
| 9 | `objcopy` | Copia/transforma objetos (extraer secciones, cambiar formato) |
| 10 | `size` | Informa tamaños de secciones (text, data, bss) |
| 11 | `strings` | Extrae cadenas legibles del binario |
| 12 | `addr2line` | Mapea direcciones a archivo y línea de fuente |
| 13 | `c++filt` | Demangle de nombres C++ (símbolos reales de `cpp_wrapper.o`) |

## Requisitos previos

- GCC con soporte C11
- G++ con soporte C++17
- GNU Binutils (habitualmente preinstalado en Linux)
- GNU Make
- GDB, para la actividad de depuración (`sudo apt install gdb`)
