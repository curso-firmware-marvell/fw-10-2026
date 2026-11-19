# Trabajo Práctico N°5: Drivers de periféricos

La idea de este TP es construir una capa de drivers de firmware de forma incremental, validando cada módulo antes de integrar el sistema completo.

Estimación sugerida: entre 5 y 7 horas.

## Antes de empezar

Objetivos del TP:
- Implementar drivers para LED, UART, DMA y ADC.
- Mantener encapsulado el acceso a registros: solo cada driver puede tocar sus registros.
- Integrar todo en una app de demo sin accesos directos a hardware desde main.

Estructura esperada:

```text
fw/drivers/inc/
fw/drivers/src/
```

Reglas generales:
- Cada driver debe tener su .h y su .c.
- En main y en demos no debe haber escrituras directas a registros.
- Validar cada hito antes de avanzar al siguiente.
- La interfaz de cada driver es exactamente la de "Interfaz requerida" (nombres y tipos): el chequeo de la cátedra, `fw/tests/test_drivers.c`, la usa tal cual.

## Hitos

Avancen en orden. Después de cada hito: compilen, ejecuten la demo y registren evidencia en su informe.

### 1. Preparación del proyecto

Tareas:
- Crear los archivos base de los cuatro drivers:
  - led.h y led.c
  - uart.h y uart.c
  - dma.h y dma.c
  - adc.h y adc.c
- Ubicarlos en:
  - fw/drivers/inc
  - fw/drivers/src
- Sumar cada `.c` a `fw/sources.mk`, una línea por archivo (por ejemplo `SRC += drivers/src/led.c`). Un `.c` que no está en `sources.mk` no se compila ni se enlaza. Eso incluye `drivers/src/uart1.c` del punto 3.b.


Pregunta de control:
1. ¿Por qué conviene separar interfaz pública en archivos .h e implementación en archivos .c?

### 2. Driver LED

Objetivo:
- Controlar los 8 bits efectivos de LED.

Interfaz requerida:
- led_init(void)
- led_on(uint8_t idx)
- led_off(uint8_t idx)
- led_toggle(uint8_t idx)
- led_write(uint8_t value)
- led_set_mask(uint32_t mask)
- led_get_mask(void)
- led_get_status(uint8_t idx)

Reglas:
- idx válido: 0 a 7.
- Si idx es inválido, no modificar el registro.
- `led_write(value)` deja los 8 LEDs exactamente con el valor `value`: a diferencia de `led_on`, `led_off` y `led_toggle`, no conserva el estado anterior.

Chequeo mínimo:
- Encender, apagar y toggle en uno y varios bits.
- Verificar que el estado final coincida con lo esperado.

Pregunta de control:
1. ¿Qué política usaron para idx inválido y por qué?

### 3.a Driver UART0

Objetivo:
- Encapsular la transmisión por UART0.

Interfaz requerida:
- uart_write_byte(char c)
- uart_write_str(const char *s)

Reglas:
- Las dos funciones son bloqueantes: al volver, la transmisión terminó.

Chequeo mínimo:
- Enviar texto de debug por UART0.
- Verificar que no haya accesos directos a registros fuera de uart.c.


### 3.b Driver UART1 o UART SERIAL

Objetivo:
- Escribir el driver de UART1 a partir del datasheet.

Recursos:
- `fw/drivers/inc/uart1.h` (la interfaz documentada), `fw/drivers/src/uart1.c` con los cuerpos vacíos, y la aplicación de prueba `fw/tests/test_uart_serial.c` con su patrón.

Qué hacer?
- Completar en `fw/drivers/src/uart1.c`:


| Función | Qué hace |
|---|---|
| `uart1_init(baud_div)` | Programa `BAUD_DIV` y limpia los flags de error. |
| `uart1_rx_level()` | Cantidad de bytes en el FIFO RX. |
| `uart1_getc(&b)` | Lee un byte **sin bloquear**: devuelve 1 si había uno, 0 si no. |
| `uart1_putc(b)` / `uart1_write(buf, n)` | Mandan bytes; esperan si el FIFO TX está lleno. |
| `uart1_tx_idle()` | 1 si ya salió el último byte. |
| `uart1_errors()` / `uart1_clear_errors()` | Leen y limpian los flags sticky (`UART1_ERR_FRAME`, `UART1_ERR_OVERRUN`). |

Sin control de flujo: `uart1_set_rts()` es del próximo práctico (TP6).

Chequeo: 

- (`test_uart_serial.c`, ya escrita) usa su driver con un protocolo de un byte de comando: al arrancar manda `K`; `E n d0..dn-1` devuelve los mismos `n` bytes (eco); `S k` deja de leer un rato y manda `S` cuando vuelve (provoca un overrun); `R` manda `R` seguido de los flags (bit 0 framing error, bit 1 overrun) y los limpia.

- `make test_uart_serial` verifica:
  1. el firmware arranca y manda `K`;
  2. el eco de 256 bytes (`00`..`FF`) vuelve intacto, y el de bytes que se parecen al protocolo del TP;
  3. un byte con el bit de stop en 0 se detecta como *framing error* (y no llega a la aplicación);
  4. 40 bytes que llegan mientras el firmware no lee provocan *overrun*;
  5. los flags se limpian al leerlos.

Tip: `make test_uart_serial VERBOSE=1` y `make wave` para mirar `tx`, `rx` y los FIFOs en las ondas.

---

### 4. Driver DMA

Objetivo:
- Copiar bloques de memoria utilizando el bloque DMA.

Interfaz requerida:
- dma_init(void)
- dma_copy(const void *src, void *dst, uint32_t n_words)
- dma_is_done(void)

Reglas:
- El START debe quedar encapsulado en el driver.
- `dma_copy` es bloqueante: al volver, la copia terminó y `dma_is_done()` devuelve un valor distinto de 0.
- Direcciones inválidas: si el rango de `src` o de `dst` (`n_words` palabras desde la dirección) sale de la zona mapeada, `0x0000_0000`–`0x0005_FFFF` (ver `project_config.def`), `dma_copy` no inicia la transferencia y no modifica el destino. En esas direcciones el bus no responde nunca y el DMA quedaría colgado.

Chequeo mínimo:
- Copiar un arreglo origen a destino.
- Validar que destino coincide al final.
- Chequear direcciones invalidas. 

Pregunta de control:
1. ¿Qué pasa si dst y src se solapan? ¿Cómo lo documentaron?

### 5. Driver ADC

Objetivo:
- Configurar y leer muestras de ADC.

Interfaz requerida:
- adc_init(uint32_t clk_div)
- adc_start(uint8_t adc_mode)
- adc_half_done(void)
- adc_done(void)
- adc_read(uint32_t *data, uint32_t size)

Reglas:
- `adc_init` fija el divisor de reloj del muestreo (`REG_ADC_CLK_DIVIDER`); `adc_read` adquiere con el divisor ya configurado, no fija uno propio.
- `adc_read` hace una adquisición completa (dispara, espera el fin y copia las muestras) y devuelve cuántas copió: como máximo 256 (`REG_ARRAY_ADC_BUFF_SIZE`), y 0 si `data` es nulo.
- `adc_half_done()` y `adc_done()` son bloqueantes: esperan el flag correspondiente, leído con máscara.
- `REG_ADC_STATUS`: bit 0 = mitad de la captura lista, bit 1 = captura completa.
- `adc_mode` es el valor que se escribe en `REG_ADC_START`: bit 0 = inicio de captura, bit 1 = modo (0 = captura única de 256 muestras, 1 = circular: recomienza sola al terminar). `adc_read` usa captura única (`adc_start(1)`).
- La captura arranca con el flanco 0→1 del bit 0 de `REG_ADC_START`: `adc_start` tiene que escribir 0 antes de `adc_mode`. Si el registro ya valía 1 de una captura anterior no hay flanco, no arranca, y el flag de captura completa sigue en 1 con los datos viejos.

Chequeo mínimo:
- Obtener una lectura del ADC.
- Imprimir la lectura mediante el UART. 

Ejemplo: 

```c
uint32_t samples[256];
adc_read(samples, 256);
print_str("-- ADC data read --");
for (int i = 0; i < 256; i++) {
  print_dec(samples[i]);
}
```
- plotear la señal de salida cambiando el `clk_div`. Para plotear la salida puede usar el script de python de la siguiente manera:

```bash
cd /mnt/c/Users/<USER>/Curso/
python3 fw/tools/plot_adc.py
open fw/build/adc_plot.png
```

o

```py
# arr = [] -> Este debe ser el array de salida del ADC
vref = 3.3
fullscale = (2**32) - 1
volts = [x * vref / fullscale for x in arr]

plt.figure(figsize=(10, 4))
plt.plot(volts, marker='.', linewidth=1)
plt.title('ADC voltage')
plt.xlabel('Sample index')
plt.ylabel('Voltage [V]')
plt.grid(True)
plt.tight_layout()
plt.show()
``` 

### 6. Integración en app de demo

Objetivo:
- Armar una app que use todos los drivers exclusivamente por API pública.

Requisitos de demo:
- UART: mensajes de estado.
- LED: indicador de progreso.
- DMA: copia de prueba y validación.
- ADC: adquisición y lectura de muestras.

Chequeo mínimo:
- Ejecutar demo completa y validar que cada bloque se comporta como se espera. 

### 7. Cierre y validación final

Checklist final:
- Compila desde cero.
- Correr fw/tests/test_drivers.c
  - make test_drivers (tiene que terminar en [ OK ] PASS)
  - o, para ver la consola completa: make fw TARGET=test_drivers y make sim
- Correr fw/tests/test_uart_serial.c
  - make test_uart_serial (tiene que terminar en [ OK ] PASS)

## Entrega

Entregar:
1. Directorio del TP completo con código fuente.
2. Informe de implementación.

El informe debe incluir, como mínimo:
1. Paso a paso de los hitos realizados.
2. Decisiones técnicas relevantes.
3. Errores encontrados y cómo los resolvieron.
4. Evidencia de validación por hito.


