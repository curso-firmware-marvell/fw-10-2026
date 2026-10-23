/**
 * @file    cpp_wrapper.h
 * @brief   C++ wrappers over the plain-C embedded libraries.
 *
 * Demonstrates the common firmware pattern: low-level drivers in C,
 * higher-level abstractions in C++.  The C++ layer adds type safety,
 * RAII, operator overloading, and templates while the underlying
 * work is still done by the C functions.
 *
 * This file produces mangled C++ symbols, useful for exercising
 * @c c++filt, @c nm @c --demangle, and related binutils.
 */

#ifndef CPP_WRAPPER_H
#define CPP_WRAPPER_H

#include <cstdint>
#include <cstddef>
#include <cstdio>

extern "C" {
#include "mathlib.h"
#include "strutils.h"
}

/**
 * @namespace hal
 * @brief     Hardware Abstraction Layer — C++ wrappers over C drivers.
 */
namespace hal {

/**
 * @class   FixedPoint
 * @brief   Type-safe wrapper around Q16.16 fixed-point arithmetic.
 *
 * All heavy lifting (multiply, divide) delegates to the C functions
 * in @c mathlib.h.  Operator overloads provide natural syntax.
 */
class FixedPoint {
public:
    /**
     * @brief   Construct from a raw Q16.16 value.
     * @param   raw     Raw fixed-point representation (default 0).
     */
    explicit FixedPoint(int32_t raw = 0) : raw_(raw) {}

    /** @brief Construct from an integer value. */
    static FixedPoint from_int(int v)     { return FixedPoint(INT_TO_FIXED(v)); }

    /** @brief Construct from a floating-point value. */
    static FixedPoint from_float(float v) { return FixedPoint(FLOAT_TO_FIXED(v)); }

    /** @brief  Get the raw Q16.16 representation. */
    int32_t raw()        const { return raw_; }

    /** @brief  Convert to integer (truncates fractional part). */
    int     to_int()     const { return FIXED_TO_INT(raw_); }

    /** @brief  Convert to float. */
    float   to_float()   const { return FIXED_TO_FLOAT(raw_); }

    /** @name Arithmetic operators */
    /** @{ */
    FixedPoint operator+(const FixedPoint &o) const { return FixedPoint(raw_ + o.raw_); }
    FixedPoint operator-(const FixedPoint &o) const { return FixedPoint(raw_ - o.raw_); }
    FixedPoint operator*(const FixedPoint &o) const { return FixedPoint(fixed_mul(raw_, o.raw_)); }
    FixedPoint operator/(const FixedPoint &o) const { return FixedPoint(fixed_div(raw_, o.raw_)); }
    /** @} */

    /** @name Comparison operators */
    /** @{ */
    bool operator==(const FixedPoint &o) const { return raw_ == o.raw_; }
    bool operator<(const FixedPoint &o)  const { return raw_ <  o.raw_; }
    /** @} */

    /**
     * @brief   Print the value to stdout with an optional label.
     * @param   label   Optional prefix string (may be NULL).
     */
    void print(const char *label = nullptr) const;

private:
    fixed16_t raw_;     /**< Internal Q16.16 storage. */
};

/**
 * @class   Matrix2x2
 * @brief   Wrapper around the C @ref mat2_t providing operator overloading.
 */
class Matrix2x2 {
public:
    /** @brief Construct as identity matrix. */
    Matrix2x2();

    /**
     * @brief   Construct with explicit elements.
     * @param   a   Element (0,0).
     * @param   b   Element (0,1).
     * @param   c   Element (1,0).
     * @param   d   Element (1,1).
     */
    Matrix2x2(int32_t a, int32_t b, int32_t c, int32_t d);

    /** @brief Matrix multiplication: @c this * @p o. */
    Matrix2x2 operator*(const Matrix2x2 &o) const;

    /** @brief Compute the determinant. */
    int32_t   determinant() const;

    /**
     * @brief   Print the matrix to stdout with an optional label.
     * @param   label   Optional prefix string (may be NULL).
     */
    void      print(const char *label = nullptr) const;

    /** @brief Mutable element access. */
    int32_t &at(int r, int c)             { return m_.m[r][c]; }

    /** @brief Const element access. */
    int32_t  at(int r, int c) const       { return m_.m[r][c]; }

private:
    mat2_t m_;  /**< Underlying C matrix struct. */
};

/**
 * @class   RingBuffer
 * @brief   Static ring buffer template — compile-time capacity, no heap.
 * @tparam  T   Element type.
 * @tparam  N   Buffer capacity (number of elements).
 */
template <typename T, uint16_t N>
class RingBuffer {
public:
    RingBuffer() : head_(0), tail_(0), count_(0) {}

    /**
     * @brief   Push an item into the buffer.
     * @param   item    Element to enqueue.
     * @return  @c true on success, @c false if the buffer is full.
     */
    bool push(const T &item) {
        if (count_ >= N) return false;
        buf_[head_] = item;
        head_ = (head_ + 1) % N;
        count_++;
        return true;
    }

    /**
     * @brief   Pop an item from the buffer.
     * @param   item    Pointer to store the dequeued element.
     * @return  @c true on success, @c false if the buffer is empty.
     */
    bool pop(T *item) {
        if (count_ == 0) return false;
        *item = buf_[tail_];
        tail_ = (tail_ + 1) % N;
        count_--;
        return true;
    }

    /** @brief Current number of elements in the buffer. */
    uint16_t count()    const { return count_; }

    /** @brief Maximum capacity (compile-time constant). */
    uint16_t capacity() const { return N; }

    /** @brief Check whether the buffer is empty. */
    bool     empty()    const { return count_ == 0; }

    /** @brief Check whether the buffer is full. */
    bool     full()     const { return count_ >= N; }

private:
    T        buf_[N];       /**< Element storage.   */
    uint16_t head_;         /**< Write index.       */
    uint16_t tail_;         /**< Read index.        */
    uint16_t count_;        /**< Current occupancy. */
};

/**
 * @class   StringBuffer
 * @brief   RAII wrapper around the C @ref strbuf_t.
 */
class StringBuffer {
public:
    /** @brief Construct an empty string buffer. */
    StringBuffer();

    /**
     * @brief   Construct with an initial string.
     * @param   initial NUL-terminated initial contents.
     */
    explicit StringBuffer(const char *initial);

    /** @brief Append a C string. */
    StringBuffer &append(const char *s);

    /** @brief Append the decimal representation of an integer. */
    StringBuffer &append(int32_t n);

    /** @brief Get a read-only C string pointer. */
    const char *c_str() const;

    /** @brief Current string length. */
    size_t      length() const;

    /** @brief Reset to empty. */
    void        clear();

    /** @brief Convert contents to uppercase in place. */
    void to_upper();

    /** @brief Convert contents to lowercase in place. */
    void to_lower();

    /** @brief Reverse the string in place. */
    void reverse();

private:
    strbuf_t buf_;  /**< Underlying C string buffer. */
};

/**
 * @class   Peripheral
 * @brief   Abstract base class for peripherals (demonstrates virtual dispatch).
 */
class Peripheral {
public:
    virtual ~Peripheral() = default;

    /**
     * @brief   Initialise the peripheral.
     * @param   config  Configuration value (e.g. baud rate).
     */
    virtual void     init(uint32_t config) = 0;

    /**
     * @brief   Send a data buffer.
     * @param   data    Pointer to the payload.
     * @param   len     Number of bytes.
     */
    virtual void     send(const uint8_t *data, size_t len) = 0;

    /** @brief  Read the current status register. */
    virtual uint32_t status() const = 0;

    /** @brief  Get the peripheral's human-readable name. */
    virtual const char *name() const = 0;
};

/**
 * @class   UartPeripheral
 * @brief   Concrete UART implementation of @ref Peripheral.
 */
class UartPeripheral : public Peripheral {
public:
    UartPeripheral();
    void     init(uint32_t baudrate) override;
    void     send(const uint8_t *data, size_t len) override;
    uint32_t status() const override;
    const char *name() const override;

private:
    uint32_t cr1_;      /**< Control register 1 shadow.    */
    uint32_t sr_;       /**< Status register shadow.       */
    uint32_t brr_;      /**< Baud-rate register shadow.    */
    uint32_t tx_count_; /**< Cumulative TX byte count.     */
};

/**
 * @namespace hal::trig
 * @brief     Trigonometric helpers using the C sine LUT.
 */
namespace trig {
    /**
     * @brief   Sine via LUT (wraps @ref math_sin).
     * @param   degrees Angle in degrees.
     * @return  FixedPoint result.
     */
    FixedPoint sin(int32_t degrees);

    /**
     * @brief   Cosine via LUT (wraps @ref math_cos).
     * @param   degrees Angle in degrees.
     * @return  FixedPoint result.
     */
    FixedPoint cos(int32_t degrees);
}

} // namespace hal

/**
 * @brief   C-callable entry point that runs the full C++ demo section.
 *
 * Exercises FixedPoint, Matrix2x2, RingBuffer, StringBuffer,
 * virtual Peripheral dispatch, and trig namespace.
 */
#ifdef __cplusplus
extern "C" {
#endif

void run_cpp_demo(void);

#ifdef __cplusplus
}
#endif

#endif /* CPP_WRAPPER_H */
