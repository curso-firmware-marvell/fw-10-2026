# Bloque 3 — Compilar un proyecto firmware

Material del curso para aprender a **organizar el código**, **compilar**, **enlazar** e **inspeccionar** binarios de un proyecto en C orientado a embebidos en Linux (toolchain GNU).

## Contenido

| Directorio | Propósito |
|------------|-----------|
| [`actividades_de_clase/`](actividades_de_clase/) | **Actividades de clase** — proyecto de referencia completo con Makefile, demos y script de prueba de binutils |
| [`trabajo_practico/`](trabajo_practico/) | **Trabajo práctico N°3** — plantilla mínima, spec distinta a la de las actividades de clase; verificación con `verify_lab.sh` |

## Ruta de aprendizaje

1. **Explorar la referencia** — leer [`actividades_de_clase/README.md`](actividades_de_clase/README.md), compilar y ejecutar la demo:

   ```bash
   cd actividades_de_clase
   make all
   ./build/binutils_demo
   ```

2. **Inspeccionar con binutils** — ejecutar el script guiado (todas las herramientas o una a la vez):

   ```bash
   bash test_binutils.sh          # las 13 herramientas
   bash test_binutils.sh nm       # solo símbolos
   bash test_binutils.sh objdump  # solo desensamblado
   ```

3. **Trabajo práctico** — compilar y enlazar en [`trabajo_practico/`](trabajo_practico/) (~5–7 h); entrega con `make verify`.

## Arquitectura del proyecto

Ambos proyectos comparten la misma organización por capas:

```
src/
├── app/     Punto de entrada de la aplicación (main.c)
├── lib/     Código reutilizable e independiente de la plataforma
├── hal/     Abstracción de hardware (periféricos simulados)
└── demo/    Código de prueba / demostración
```

El proyecto de referencia añade `cpp/` (envoltorio C++ para demos de símbolos mangled) y `asm/` (ensamblador para demos de `as`). El laboratorio añade `src/platform/`: la salida de consola (`print.h`), con un backend para la PC en `src/platform/host/`.

## Requisitos previos

- GCC (C11) y G++ (C++17) — solo en el proyecto de referencia
- GNU Make
- GNU Binutils (`as`, `ld`, `nm`, `objdump`, `readelf`, `ar`, `strip`, `size`, etc.)

En Ubuntu/Debian:

```bash
sudo apt install build-essential
```

## Esquema sugerido para la clase

| Bloque | Tema | Dónde |
|--------|------|-------|
| 1 | Estructura del proyecto y separación de responsabilidades | `actividades_de_clase/README.md` — Estructura del proyecto |
| 2 | Recorrido del Makefile: compilar, enlazar, biblioteca estática | `actividades_de_clase/Makefile` |
| 3 | Inspección en vivo con binutils | `actividades_de_clase/test_binutils.sh` |
| 4 | Práctica guiada (hitos 1–10) | `trabajo_practico/README.md` |
| 5 | Biblioteca estática, `-O2`, binutils, `verify_lab.sh` | hitos 11–14 |

## Material relacionado

La [raíz del repositorio](../../) contiene la plataforma de simulación FPGA RISC-V ([Makefile](../../Makefile), [RTL](../../rtl/), [firmware](../../fw/)). El laboratorio TP3 conserva su propio backend de consola en `src/platform/`; no depende de la plataforma FPGA ni integra su biblioteca dentro del firmware.
