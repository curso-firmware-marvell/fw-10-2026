#ifndef CMD_PROTOCOL_DEFS_H
#define CMD_PROTOCOL_DEFS_H

/* Constantes del protocolo del hito 2 (ver la consigna, secciones de protocolo y comandos).
 * Solo constantes: la logica (sincronizacion, CRC, validaciones, comandos) la escribe cada grupo. */

#define PKT_HEADER_0        0xA5
#define PKT_HEADER_1        0x5A
#define PKT_FOOTER_0        0xC3
#define PKT_FOOTER_1        0x3C

#define OP_ADC              0x01
#define OP_DMA              0x02
#define OP_MEM_READ         0x03
#define OP_MEM_WRITE        0x04
#define OP_LED              0x05
#define OP_RESPONSE_FLAG    0x80    /* OPCODE_RESP = OPCODE_REQUEST | 0x80 */

#define LED_OP_SET          0x00
#define LED_OP_GET          0x01

#define STATUS_OK                   0x00
#define STATUS_ERROR_CRC            0x01
#define STATUS_ERROR_LENGTH         0x02
#define STATUS_ERROR_UNKNOWN_OPCODE 0x03
#define STATUS_ERROR_INVALID_PARAM  0x04
#define STATUS_ERROR_HW             0x05

#endif /* CMD_PROTOCOL_DEFS_H */
