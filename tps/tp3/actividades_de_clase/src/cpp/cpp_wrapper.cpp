/**
 * @file    cpp_wrapper.cpp
 * @brief   Implementation of C++ wrappers and the run_cpp_demo() entry point.
 *
 * @see     cpp_wrapper.h for class declarations and design rationale.
 */

#include "cpp_wrapper.h"
#include <cstdio>
#include <cstring>

namespace hal {

/* ── FixedPoint ────────────────────────────────────────────────── */

void FixedPoint::print(const char *label) const {
    if (label)
        std::printf("[fixed++] %s = %.4f\n", label, to_float());
    else
        std::printf("[fixed++] %.4f\n", to_float());
}

/* ── Matrix2x2 ─────────────────────────────────────────────────── */

Matrix2x2::Matrix2x2() {
    mat2_identity(&m_);
}

Matrix2x2::Matrix2x2(int32_t a, int32_t b, int32_t c, int32_t d) {
    m_.m[0][0] = a; m_.m[0][1] = b;
    m_.m[1][0] = c; m_.m[1][1] = d;
}

Matrix2x2 Matrix2x2::operator*(const Matrix2x2 &o) const {
    Matrix2x2 result;
    mat2_multiply(&result.m_, &m_, &o.m_);
    return result;
}

int32_t Matrix2x2::determinant() const {
    return mat2_determinant(&m_);
}

void Matrix2x2::print(const char *label) const {
    if (label)
        std::printf("[mat++] %s:\n", label);
    mat2_print(&m_);
}

/* ── StringBuffer ──────────────────────────────────────────────── */

StringBuffer::StringBuffer() {
    strbuf_init(&buf_);
}

StringBuffer::StringBuffer(const char *initial) {
    strbuf_init(&buf_);
    strbuf_set(&buf_, initial);
}

StringBuffer &StringBuffer::append(const char *s) {
    strbuf_append(&buf_, s);
    return *this;
}

StringBuffer &StringBuffer::append(int32_t n) {
    strbuf_append_int(&buf_, n);
    return *this;
}

const char *StringBuffer::c_str() const {
    return strbuf_cstr(&buf_);
}

size_t StringBuffer::length() const {
    return buf_.len;
}

void StringBuffer::clear() {
    strbuf_clear(&buf_);
}

void StringBuffer::to_upper() {
    char tmp[STR_MAX_LEN];
    str_to_upper(tmp, buf_.data, STR_MAX_LEN);
    strbuf_set(&buf_, tmp);
}

void StringBuffer::to_lower() {
    char tmp[STR_MAX_LEN];
    str_to_lower(tmp, buf_.data, STR_MAX_LEN);
    strbuf_set(&buf_, tmp);
}

void StringBuffer::reverse() {
    str_reverse(buf_.data, buf_.len);
}

/* ── UartPeripheral ────────────────────────────────────────────── */

UartPeripheral::UartPeripheral()
    : cr1_(0), sr_(0), brr_(0), tx_count_(0) {}

void UartPeripheral::init(uint32_t baudrate) {
    brr_ = 8000000U / baudrate;
    cr1_ = bit_set(0, 13);
    cr1_ = bit_set(cr1_, 3);
    cr1_ = bit_set(cr1_, 2);
    sr_  = 0;
    tx_count_ = 0;
}

void UartPeripheral::send(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        (void)data[i];
        tx_count_++;
    }
    sr_ = bit_set(sr_, 6);
}

uint32_t UartPeripheral::status() const {
    return sr_;
}

const char *UartPeripheral::name() const {
    return "UART1";
}

/* ── Trig namespace ────────────────────────────────────────────── */

namespace trig {

FixedPoint sin(int32_t degrees) {
    return FixedPoint(math_sin(degrees));
}

FixedPoint cos(int32_t degrees) {
    return FixedPoint(math_cos(degrees));
}

} // namespace trig

} // namespace hal

/* ── C++ demo (callable from main.c via extern "C") ────────────── */

extern "C" void run_cpp_demo(void) {
    std::printf("\n--- C++ Wrapper Demo ---\n");

    /* FixedPoint */
    hal::FixedPoint a = hal::FixedPoint::from_float(2.5f);
    hal::FixedPoint b = hal::FixedPoint::from_float(4.0f);
    hal::FixedPoint c = a * b;
    a.print("2.5");
    b.print("4.0");
    c.print("2.5 * 4.0");
    (a + b).print("2.5 + 4.0");

    /* Matrix */
    hal::Matrix2x2 ma(1, 2, 3, 4);
    hal::Matrix2x2 mb(5, 6, 7, 8);
    hal::Matrix2x2 mc = ma * mb;
    ma.print("A");
    mb.print("B");
    mc.print("A * B");
    std::printf("[mat++] det(A*B) = %d\n", (int)mc.determinant());

    /* Template ring buffer with typed data */
    hal::RingBuffer<int32_t, 8> int_ring;
    for (int32_t i = 100; i < 106; i++)
        int_ring.push(i);
    std::printf("[ring++] count=%u, capacity=%u\n",
                (unsigned)int_ring.count(), (unsigned)int_ring.capacity());
    int32_t val;
    std::printf("[ring++] popped:");
    while (int_ring.pop(&val))
        std::printf(" %d", (int)val);
    std::printf("\n");

    /* StringBuffer */
    hal::StringBuffer sb("Firmware v");
    sb.append("2.0 build ");
    sb.append((int32_t)42);
    std::printf("[str++] buffer  = '%s' (len=%zu)\n", sb.c_str(), sb.length());
    sb.to_upper();
    std::printf("[str++] upper   = '%s'\n", sb.c_str());
    sb.reverse();
    std::printf("[str++] reverse = '%s'\n", sb.c_str());

    /* Virtual dispatch through Peripheral* */
    hal::UartPeripheral uart;
    hal::Peripheral *periph = &uart;
    periph->init(115200);
    const uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    periph->send(payload, sizeof(payload));
    std::printf("[periph] %s: SR=0x%08X\n", periph->name(), (unsigned)periph->status());

    /* Trig via namespace */
    hal::trig::sin(45).print("sin(45)");
    hal::trig::cos(60).print("cos(60)");
}
