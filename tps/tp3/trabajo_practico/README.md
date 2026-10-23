# Trabajo Práctico N°3 : Compilación y Enlace

La idea de este TP es armar un proyecto en C "de embebido" de a poco, módulo por módulo, e ir integrándolo con un Makefile hasta terminar con un ejecutable enlazado contra una biblioteca estática. Los periféricos son dummy, así que todo corre en su PC.

Calculen entre 5 y 7 horas.

### Antes de empezar

Necesitan gcc y make:

```bash
sudo apt install build-essential
```

El proyecto arranca así:

```
trabajo_practico/
├── Makefile          # plantilla: solo compila main.c
├── verify_lab.sh     # verificación de la entrega
└── src/
    ├── app/main.c    # Hello World
    ├── lib/          # mathlib, strutils
    ├── hal/          # hal_uart, hal_ring, hal_timer
    ├── demo/         # demo_math, demo_str, demo_embedded
    └── platform/     # salida de consola (ya viene hecho)
        ├── print.h, print.c
        └── host/     # print_chr() sobre stdout
```

Algunas reglas:

- No toquen los headers (`.h`): son la API que tienen que respetar. Los `.c` de `lib/`, `hal/` y `demo/` están vacíos y los completan ustedes.
- `src/platform/` no se modifica. Para imprimir usen sus funciones (`print_str`, `print_dec`, `print_hex`, `print_chr`) y no `printf`. Más adelante este mismo código se va a correr en otra plataforma, y lo único que debería cambiar es la implementación de la salida.
- En el Makefile ya están las reglas para compilar y enlazar; ustedes solo van agregando archivos a `SRCS` y `LIB_SRCS`. Cada vez que cambien `SRCS`, `LIB_SRCS` o `USE_STATIC_LIB` hagan un `make clean`.

### Ojo con las actividades de clase

El proyecto de [`../actividades_de_clase/`](../actividades_de_clase/) que vimos en clase tiene módulos parecidos, pero con otros parámetros. Si copian de ahí, la verificación va a fallar. Los valores que valen son los de los headers:

| Parámetro | Constante | Valor |
|-----------|-----------|-------|
| Reloj del periférico UART | `UART_PERIPH_CLK_HZ` | 16 MHz |
| Valor inicial del CRC-8 | `STR_CRC8_INIT` | `0xFF` (polinomio `0x07`) |
| Capacidad del buffer circular | `RING_BUF_SIZE` | 32 bytes |

### Hitos

Vayan en orden. Después de cada hito compilen con `make`, corran `./build/building_process_demo` y fíjense que la salida sea la que se pide.

**1. Compilar lo que hay.** Compilen y ejecuten el proyecto tal cual viene; tiene que imprimir `Hello, World!`. Miren qué se generó:

```bash
nm build/app/main.o
size build/building_process_demo
```

`main.o` usa `print_str` pero no la define. ¿En qué objeto de `build/platform/` está?

**2. Biblioteca matemática.** Primero agreguen `-Werror` a `CFLAGS`: de acá en adelante no se aceptan warnings. Implementen `math_factorial` y `math_gcd` en `src/lib/mathlib.c`, agréguenlo a `SRCS` y desde `main.c` impriman:

```
factorial(5) = 120
gcd(48, 18) = 6
```

Con `nm build/lib/mathlib.o` verifiquen que las dos funciones aparecen como `T`.

**3. Más matemática.** Sumen `math_isqrt` (raíz cuadrada entera) y `math_is_prime` sin romper lo anterior:

```
isqrt(17) = 4
is_prime(17) = 1
is_prime(18) = 0
```

**4. Strings.** Implementen las funciones `strbuf_*` de `src/lib/strutils.c` y agréguenlo a `SRCS`. Nada de memoria dinámica, y ninguna función puede escribir más allá de `STR_MAX_LEN`. Con `strbuf_set` y `strbuf_append_int` armen e impriman `value=42`.

**5. Trim y CRC-8.** Implementen `str_trim` y `str_crc8` (que arranca en `STR_CRC8_INIT`). Recorten `"  hello  "`, calculen el CRC de `{0x01, 0x02, 0x03, 0x04}` e impriman lo siguiente (el CRC con `print_hex(crc, 2)`):

```
trimmed: 'hello'
crc8: 0x32
```

**6. Driver UART.** La UART es una estructura de registros (`uart_regs_t`), no hay hardware de verdad. En `src/hal/hal_uart.c`:

- `uart_init` calcula `BRR = UART_PERIPH_CLK_HZ / baudrate`, prende los bits 13, 3 y 2 de `CR1` con `bit_set()` y pone `SR` y `DR` en cero.
- `uart_send_byte` escribe el byte en `DR` y prende el bit 6 de `SR` (transmisión completa).
- `uart_send` manda un buffer entero usando `uart_send_byte`.

Inicialicen a 115200 baudios, manden `"OK"` e impriman los registros con `print_hex(valor, 8)`:

```
uart BRR=138 SR=0x00000040 DR=0x0000004B
```

**7. Buffer circular.** `ring_init`, `ring_push` y `ring_pop` van en `src/hal/hal_ring.c`. `ring_push` devuelve `-1` si el buffer está lleno y `ring_pop` devuelve `-1` si está vacío. Encolen de `'A'` a `'J'`, desencolen todo e impriman `ring: ABCDEFGHIJ`.

**8. Timer.** Para este no hay nada parecido en las actividades de clase, así que sale del header. `timer_init` pone el contador en cero, `timer_tick` suma 1 ms y `timer_get_ms` devuelve lo acumulado.

**9. Juntar todo.** En `demo_embedded.c` armen una trama de 6 bytes: `0xAA` de sincronismo, el payload `{0x01, 0x02, 0x03, 0x04}` y al final el CRC-8 del payload. Pásenla entera por el buffer circular y, del otro lado, validen que el primer byte sea `0xAA` y que el CRC recalculado coincida con el último. Impriman `frame: ok crc=0x32`, o `frame: error` si algo no cierra. Después inicialicen el timer, llamen tres veces a `timer_tick()` e impriman `timer_ms: 3`.

**10. Limpiar el main.** Muevan el código de prueba de los hitos 2 a 9 a `demo_math()`, `demo_str()` y `demo_embedded()`, agreguen los tres archivos de `src/demo/` a `SRCS` y dejen en `main()` solo las tres llamadas. La salida tiene que quedar igual.

**11. Biblioteca estática.** Pongan `USE_STATIC_LIB = 1` y pasen todas las fuentes de `lib/` y `hal/` (sí, también `hal_timer.c`) de `SRCS` a `LIB_SRCS`; en `SRCS` quedan solo `app/` y `demo/`. Después:

```bash
make clean && make
ar t build/libutils.a
nm --defined-only build/libutils.a
```

**12. Optimización.**

```bash
make opt
size build/building_process_demo build/building_process_demo_opt
```

Armen una tabla con `text`, `data` y `bss` de los dos ejecutables y expliquen por qué cambian.

**13. `--version`.** Definan `PROGRAM_NAME` y `PROGRAM_VERSION` en `main.c`. Si el programa recibe `--version`, imprime nombre y versión y termina sin correr las demos. Chequeen que las cadenas quedaron en el binario:

```bash
./build/building_process_demo --version
strings build/building_process_demo | grep building-process
```

**14. Preguntas.** Con `nm`, `objdump`, `readelf` y `size`, respondan:

1. Tres símbolos de `mathlib.o` y su tipo según `nm`.
2. ¿Qué símbolos usa `demo_embedded.o` sin definirlos? ¿Dónde se resuelven?
3. ¿Cuál es la sección más grande del ejecutable?
4. Después del hito 11, ¿en qué miembro de `libutils.a` está `math_factorial`?
5. ¿Por qué la aplicación necesita `-lutils` para enlazar, pero `libutils.a` no necesita a la aplicación?

### Entrega

Antes de entregar corran:

```bash
make verify
```

El script compila desde cero, revisa que no usen `printf`, compara la salida del programa y se fija que `hal_timer.o` esté en `libutils.a`. Tiene que pasar todo.

Entreguen el directorio `trabajo_practico/` completo y un informe con la tabla del hito 12 y las respuestas del hito 14.

### Si algo no anda

- `undefined reference to ...` al enlazar: falta un `.c` en `SRCS` o `LIB_SRCS`, o el nombre de la función no es el del header.
- `crc8: 0xE3` en vez de `0x32`: el CRC arranca en `0x00` y no en `STR_CRC8_INIT`.
- `BRR=69` en vez de `138`: usaron un reloj de 8 MHz (el de las actividades de clase).
- `make verify` no encuentra `libutils.a`: todavía no hicieron el hito 11.
- Cambian algo y no se refleja: hay objetos viejos en `build/`, hagan `make clean`.
