/* Hito 2: procesador de comandos sobre UART1.
 *
 * Completar segun la consigna (tps/tp6.md):
 *   - sincronizarse al flujo de bytes y detectar tramas validas (HEADER, LENGTH, CRC, FOOTER);
 *     el CRC se calcula con str_crc8() de lib/inc/strutils.h: no hay que reescribirlo
 *   - ejecutar el comando SOLO cuando la trama entera llego y es correcta
 *   - ante una trama corrupta: descartarla entera, no ejecutarla y responder el error que
 *     corresponde (ERROR_CRC, ERROR_LENGTH; ver 3.5 de la consigna)
 *   - CONTROL DE FLUJO (hito 1): frenar al host con RTS antes de cada tramo en que no se lee
 *     la UART (verificar el CRC de una trama larga, ejecutar el comando, mandar la respuesta);
 *     si no, el FIFO RX se llena y se pierden bytes (el test lo prueba con tramas largas
 *     pegadas). Despues de bajar RTS todavia pueden llegar hasta 3 bytes: bajarlo con margen
 *   - decodificar OPCODE y parametros, y ejecutar el comando con los drivers
 *   - validar direcciones ANTES de tocar el bus (consigna 3.7): una direccion invalida
 *     cuelga el core, y REG_MCU_RESET no se puede escribir. Las regiones validas estan en
 *     config.h (ADDR_START_* / ADDR_SIZE_*, generado desde project_config.def)
 *   - recuperarse de tramas incompletas (dejan de llegar bytes) sin colgarse: para medir el
 *     silencio usen clock_cycles() / clock_elapsed() de lib/inc/clock.h (consigna, 3.6)
 *
 * El diseno interno es libre. Pueden agregar modulos propios (.c y .h) en esta carpeta (app/):
 * hay que listar cada .c en fw/sources.mk (SRC += app/mi_modulo.c), igual que los drivers.
 *
 * Se prueba con (desde la raiz del repo):
 *      make test_main               compila este main, lo simula y corre fw/tests/main.txt
 *      make test_main VERBOSE=1     ademas imprime cada byte que pasa por UART1
 * Constantes del protocolo: cmd_protocol_defs.h (en esta carpeta). Driver: drivers/inc/uart1.h
 * (el del TP5; uart1_set_rts() es del hito 1).
 */
#include "uart1.h"
#include "cmd_protocol_defs.h"

void main(void)
{
    REG_FW_STATUS = 0;
    uart1_init(UART1_SIM_BAUD_DIV);
    uart1_set_rts(1);

    for (;;) {
        /* TODO hito 2 */
    }
}
