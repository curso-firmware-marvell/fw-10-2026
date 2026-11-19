/* Verificacion de la catedra de los drivers. No modificar. */
#include "includes.h"
#include "led.h"
#include "dma.h"
#include "uart.h"
#include "adc.h"

typedef unsigned int u;
typedef volatile unsigned int vu;

static u z = 0x2F6E2B1Du;
static u f, e;
static vu A[320], B[320], C[320], G[1100], H[1100];
static uint32_t X[300];

static u q(void)
{
    z ^= z << 13; z ^= z >> 17; z ^= z << 5;
    return z;
}

static void k(u i, int c)
{
    if (c)
        return;
    char m[4] = { 'T', (char)('0' + i / 10), (char)('0' + i % 10), 0 };
    e = 1;
    if (i < 32)
        f |= 1u << i;
    print_str(m);
}

#define M (REG_LED & 0xFFu)

static void w0(void)
{
    static const u v[] = { 0x00, 0xFF, 0x55, 0xAA, 0x01, 0x80 };
    static const u o[] = { 8, 9, 31, 200, 255 };
    u i, j, m, s, ok;

    led_init();
    k(0, led_get_mask() == 0 && M == 0);

    ok = 1; m = 0;
    for (i = 0; i < 8; i++) {
        led_on(i); m |= 1u << i;
        ok &= (M == m) && (led_get_mask() == m) && (led_get_status(i) == 1);
    }
    k(1, ok);

    ok = 1;
    for (i = 8; i--; ) {
        led_off(i); m &= ~(1u << i);
        ok &= (M == m) && (led_get_mask() == m) && (led_get_status(i) == 0);
    }
    k(2, ok);

    ok = 1; led_set_mask(0xA5); m = 0xA5;
    for (j = 0; j < 2; j++)
        for (i = 0; i < 8; i++) {
            led_toggle(i); m ^= 1u << i;
            ok &= (M == m) && (led_get_mask() == m);
        }
    k(3, ok);

    ok = 1;
    for (j = 0; j < 6; j++) {
        led_set_mask(v[j]);
        ok &= (M == v[j]) && (led_get_mask() == v[j]);
        for (i = 0; i < 8; i++)
            ok &= (led_get_status(i) == ((v[j] >> i) & 1u));
    }
    k(4, ok);

    ok = 1; led_set_mask(0x3C);
    for (j = 0; j < 5; j++) {
        led_on(o[j]); led_off(o[j]); led_toggle(o[j]);
        ok &= (M == 0x3C) && (led_get_mask() == 0x3C);
    }
    k(5, ok);

    ok = 1; m = 0; led_set_mask(0);
    for (j = 0; j < 32; j++) {
        i = q() % 8; s = q() % 3;
        if (s == 0) { led_on(i); m |= 1u << i; }
        else if (s == 1) { led_off(i); m &= ~(1u << i); }
        else { led_toggle(i); m ^= 1u << i; }
        ok &= (M == m) && (led_get_mask() == m);
    }
    k(6, ok);

    ok = 1;
    for (j = 0; j < 6; j++) {
        led_set_mask(0xFF);
        led_write(v[j]);
        ok &= (M == v[j]) && (led_get_mask() == v[j]);
        for (i = 0; i < 8; i++)
            ok &= (led_get_status(i) == ((v[j] >> i) & 1u));
    }
    k(31, ok);
    led_set_mask(0);
}

static void w1(void)
{
    static const u n[] = { 1, 2, 3, 8, 31, 64, 128, 255 };
    u i, j, c, ok1 = 1, ok2 = 1, ok3 = 1, ok4 = 1;

    dma_init();
    for (j = 0; j < 8; j++) {
        c = n[j];
        for (i = 0; i < 320; i++) { A[i] = q(); C[i] = A[i]; B[i] = 0xA5A50000u ^ i; }
        dma_copy((const void *)&A[8], (void *)&B[8], c);
        for (i = 0; i < c; i++)
            ok1 &= (B[8 + i] == A[8 + i]);
        for (i = 0; i < 8; i++)
            ok2 &= (B[i] == (0xA5A50000u ^ i));
        for (i = 8 + c; i < 320; i++)
            ok2 &= (B[i] == (0xA5A50000u ^ i));
        for (i = 0; i < 320; i++)
            ok3 &= (A[i] == C[i]);
        ok4 &= (dma_is_done() != 0);
    }
    k(7, ok1); k(8, ok2); k(9, ok3); k(10, ok4);

    for (i = 0; i < 1100; i++) { G[i] = q(); H[i] = 0; }
    dma_copy((const void *)&G[0], (void *)&H[0], 1000);
    ok1 = (dma_is_done() != 0);
    for (i = 0; i < 1000; i++) ok1 &= (H[i] == G[i]);
    for (i = 1000; i < 1100; i++) ok1 &= (H[i] == 0);
    k(13, ok1);

    ok1 = 1;
    for (i = 0; i < 320; i++) { A[i] = q(); B[i] = 0; C[i] = 0; }
    dma_copy((const void *)&A[0], (void *)&B[0], 40);
    dma_copy((const void *)&A[100], (void *)&C[0], 40);
    for (i = 0; i < 40; i++)
        ok1 &= (B[i] == A[i]) && (C[i] == A[100 + i]);
    k(11, ok1);

    ok1 = 1;
    for (i = 0; i < 320; i++) { A[i] = q(); B[i] = 0; }
    dma_copy((const void *)&A[0], (void *)&B[0], 16);
    dma_copy((const void *)&A[50], (void *)&B[0], 16);
    for (i = 0; i < 16; i++)
        ok1 &= (B[i] == A[50 + i]);
    k(12, ok1);
}

static void w2(void)
{
    static const unsigned char by[] = { 0x41, 0x7A, 0xE9, 0x5A, 0x30 };
    static const u L[] = { 64, 1, 17, 2, 5, 33 };
    char s[80];
    u i, j, l, ok1 = 1, ok2 = 1, ok3 = 1;

    for (j = 0; j < 5; j++) {
        uart_write_byte((char)by[j]);
        ok1 &= (REG_UART0_SIZE_TO_TRANSMIT == 1) && (REG_ARRAY_UART0_BUFF_OUT[0] == by[j]);
        ok3 &= (REG_UART0_TRANSMISION_DONE == 1);
    }
    k(14, ok1);

    ok1 = 1;
    for (j = 0; j < 6; j++) {
        l = L[j];
        for (i = 0; i < l; i++)
            s[i] = (char)('a' + q() % 26);
        s[l] = 0;
        uart_write_str(s);
        ok1 &= (REG_UART0_SIZE_TO_TRANSMIT == l);
        for (i = 0; i < l; i++)
            ok2 &= (REG_ARRAY_UART0_BUFF_OUT[i] == (u)(unsigned char)s[i]);
        ok3 &= (REG_UART0_TRANSMISION_DONE == 1);
    }
    k(15, ok1); k(16, ok2);

    ok1 = 1;
    uart_write_str("xyz"); uart_write_byte('Q');
    ok1 &= (REG_UART0_SIZE_TO_TRANSMIT == 1) && (REG_ARRAY_UART0_BUFF_OUT[0] == 'Q');
    uart_write_str("mn");
    ok1 &= (REG_UART0_SIZE_TO_TRANSMIT == 2) && (REG_ARRAY_UART0_BUFF_OUT[1] == 'n');
    uart_write_str("");
    ok3 &= (REG_UART0_TRANSMISION_DONE == 1);
    k(17, ok1); k(18, ok3);
}

static int w3a(void)
{
    u i, d = 0;
    for (i = 1; i < 256; i++)
        d += (X[4 + i] != X[3 + i]);
    return d >= 64;
}

static void w3(void)
{
    static const u n[] = { 1, 16, 64, 100, 256 };
    static const u dv[] = { 1, 2, 5, 255 };
    u i, j, r, ok1 = 1, ok2 = 1, ok3 = 1;

    adc_init(3);
    for (j = 0; j < 5; j++) {
        for (i = 0; i < 300; i++) X[i] = 0xC0DE0000u | i;
        r = adc_read(&X[4], n[j]);
        ok1 &= (r == n[j]);
        for (i = 0; i < 4; i++) ok2 &= (X[i] == (0xC0DE0000u | i));
        for (i = 4 + n[j]; i < 300; i++) ok2 &= (X[i] == (0xC0DE0000u | i));
    }
    k(19, ok1); k(21, ok2);

    for (i = 0; i < 300; i++) X[i] = 0xC0DE0000u | i;
    r = adc_read(&X[4], 260);
    k(20, r == REG_ARRAY_ADC_BUFF_SIZE);
    ok2 = 1;
    for (i = 4 + REG_ARRAY_ADC_BUFF_SIZE; i < 300; i++) ok2 &= (X[i] == (0xC0DE0000u | i));
    k(22, ok2);

    k(23, adc_read(0, 8) == 0);

    ok1 = 1;
    r = adc_read(&X[4], 256);
    for (i = 0; i < 256; i++) ok1 &= (X[4 + i] == REG_ARRAY_ADC_BUFF[i]);
    k(24, ok1 && r == 256);
    k(26, w3a());
    adc_read(&X[4], 1);
    k(25, (REG_ADC_STATUS & 2u) != 0);

    for (j = 0; j < 3; j++) {
        for (i = 0; i < 300; i++) X[i] = 0;
        r = adc_read(&X[4], 256);
        ok3 &= (r == 256) && w3a();
        for (i = 0; i < 256; i++) ok3 &= (X[4 + i] == REG_ARRAY_ADC_BUFF[i]);
    }
    k(27, ok3);

    adc_start(1);
    k(29, (REG_ADC_STATUS & 2u) == 0);
    adc_done();
    k(30, (REG_ADC_STATUS & 2u) != 0);

    ok1 = 1;
    for (j = 0; j < 4; j++) {
        adc_init(dv[j]);
        ok1 &= (REG_ADC_CLK_DIVIDER == dv[j]);
    }
    k(28, ok1);
    adc_init(3);
}

/* DMA con direcciones invalidas. Es lo ultimo que se prueba: un dma_copy sin chequeo se cuelga
 * (el bus no contesta fuera del mapa) y la simulacion termina por TIMEOUT. */
static void w4(void)
{
    const void *bad = (const void *)0x00100000u;
    u i, ok1 = 1, ok2 = 1, ok3 = 1, ok4 = 1;

    for (i = 0; i < 320; i++) { A[i] = q(); C[i] = A[i]; B[i] = 0xA5A50000u ^ i; }

    dma_copy(bad, (void *)&B[0], 16);
    for (i = 0; i < 320; i++)
        ok1 &= (B[i] == (0xA5A50000u ^ i));
    k(32, ok1);

    dma_copy((const void *)&A[0], (void *)bad, 16);
    for (i = 0; i < 320; i++)
        ok2 &= (A[i] == C[i]);
    k(33, ok2);

    dma_copy((const void *)0x0005FFF8u, (void *)&B[0], 4);
    for (i = 0; i < 320; i++)
        ok3 &= (B[i] == (0xA5A50000u ^ i));
    k(34, ok3);

    dma_copy((const void *)&A[0], (void *)&B[0], 32);
    for (i = 0; i < 32; i++)
        ok4 &= (B[i] == A[i]);
    ok4 &= (dma_is_done() != 0);
    k(35, ok4);
}

int main(void)
{
    u i;

    REG_FW_STATUS = 0;
    REG_FW_REPORT = 0;
    f = 0;
    e = 0;

    w0();
    w1();
    w2();
    w3();
    w4();
    REG_FW_REPORT = f;

    adc_read(&X[4], 256);
    print_str("-- ADC data read --");
    for (i = 0; i < 256; i++)
        print_dec(X[4 + i]);

    REG_FW_STATUS = e ? 0xBAD : 0xFF;
    return 0;
}
