# UART1 — Datasheet

Serial UART of the platform (`rtl/uart_serial.v`, instance `u_uart1` in `top.v`). Unlike UART0 (a byte buffer), UART1 behaves like a real UART: it transmits and receives **bit by bit** on a serial line, and has **FIFOs** in both directions, **error flags** and **RTS/CTS** handshake lines.

Base address: `0x4_3000`. The registers are defined in `project_config.def` and reach the firmware through `config.h` (`REG_UART1_*`).

---

## 1. Line format: 8N1

```
idle line = 1
        ┌─────┬────┬────┬────┬────┬────┬────┬────┬────┬──────┐
   1 ───┘     │ D0 │ D1 │ D2 │ D3 │ D4 │ D5 │ D6 │ D7 │      └─── 1
        START │      8 data bits (LSB first)           │ STOP
         (0)  │                                        │ (1)
        ◄─────────────────  10 bit times  ────────────────────►
```

- 1 start bit (0), 8 data bits with the **LSB first**, no parity, 1 stop bit (1).
- **One bit time = `BAUD_DIV` clocks.** A complete byte occupies the line for `10 × BAUD_DIV` clocks.
- `BAUD_DIV` must be ≥ 2. Reset value: `UART1_SIM_BAUD_DIV` (32, i.e. 320 clocks per byte). The other end of the line must use the same bit time.
- Changing `BAUD_DIV` while a byte is being transmitted or received gives undefined results.

## 2. Block diagram

```
 REG_UART1_TX_DATA ───► TX FIFO (16) ───► serializer ───► tx
                                              ▲
                                       cts_n ─┘
 REG_UART1_RX_DATA ◄─── RX FIFO (16) ◄─── deserializer ◄─ rx
 REG_UART1_STATUS  ◄─── flags and levels
 REG_UART1_CTRL    ───► RTS bit ───────────────────────────► rts_n
```

## 3. Registers

| Register | Addr | Access | Description |
|---|---|---|---|
| `REG_UART1_BAUD_DIV` | `0x4_3000` | RW | Clocks per bit. Reset: `UART1_SIM_BAUD_DIV` (32). |
| `REG_UART1_CTRL`     | `0x4_3004` | RW | Control (§3.1). |
| `REG_UART1_TX_DATA`  | `0x4_3008` | RW | **Writing** pushes byte `[7:0]` into the TX FIFO (§3.4). |
| `REG_UART1_STATUS`   | `0x4_300C` | RO | Status (§3.2). |
| `REG_UART1_RX_DATA`  | `0x4_3010` | RO | **Reading** pops a byte from the RX FIFO (§3.3). |

`config.h` also defines `UART1_FIFO_DEPTH` (16) and `UART1_SIM_BAUD_DIV` (32).

### 3.1 `REG_UART1_CTRL`

| Bit | Name | Description |
|---|---|---|
| 0 | `RTS` | `1` = the firmware is ready to receive. Reset: **0**. Drives the `rts_n` line (active low). The UART never changes it by itself (manual flow control, §5). |
| 1 | `CLR_ERR` | Writing `1` clears the `OVERRUN` and `FRAME_ERR` flags. It is a pulse: it is not stored. |

- Writing `CTRL` replaces **both** bits: to clear the errors without changing RTS, write `RTS | CLR_ERR` with the RTS value that should be kept.
- Reading `CTRL` returns the last value written (including `CLR_ERR`). Before the first write the value is not guaranteed, so it is best to keep the RTS state in a firmware variable.

### 3.2 `REG_UART1_STATUS`

| Bits | Name | Description |
|---|---|---|
| 0 | `RX_NOT_EMPTY` | At least one byte is in the RX FIFO. |
| 1 | `RX_FULL` | The RX FIFO is full (16 bytes). |
| 2 | `OVERRUN` | **Sticky.** A byte arrived with the RX FIFO full and was dropped. |
| 3 | `FRAME_ERR` | **Sticky.** A byte was received with the stop bit at 0 and was dropped. |
| 4 | `TX_FULL` | The TX FIFO is full: writing `TX_DATA` now loses the byte. |
| 5 | `TX_EMPTY` | The TX FIFO is empty (the last byte may still be shifting out). |
| 6 | `TX_IDLE` | TX FIFO empty **and** the last byte has finished leaving on the line. |
| 7 | `CTS` | State of the CTS input (`1` = the other end accepts bytes). |
| 15:8 | `RX_LEVEL` | Number of bytes in the RX FIFO (0 to 16). |
| 23:16 | `TX_LEVEL` | Number of bytes in the TX FIFO (0 to 16). |

Sticky flags stay set until `CLR_ERR` is written. A new error that arrives in the same cycle as `CLR_ERR` is not lost.

### 3.3 `REG_UART1_RX_DATA`

| Bits | Description |
|---|---|
| 7:0 | Oldest byte in the RX FIFO. |
| 8 | `VALID`: `1` if there was a byte. With an empty FIFO, `VALID = 0` and the read has no effect. |

> **Reading has a side effect:** every read with `VALID = 1` **pops** the byte from the FIFO. Two consecutive reads return two different bytes. This holds for *any* read access to that address, including a generic memory read or a DMA read.

Read the register **only once** and keep the whole value (data + `VALID`) in a variable: checking `VALID` and then reading again to get the data consumes two bytes.

### 3.4 `REG_UART1_TX_DATA`

Writing pushes byte `[7:0]` into the TX FIFO. If the FIFO is full (`TX_FULL = 1`) the write is **silently lost**: wait for `TX_FULL` to clear before writing. To know that everything has left on the line, wait for `TX_IDLE`, not `TX_EMPTY`.

## 4. Behavior

### Transmission
- The serializer takes a byte from the TX FIFO when it is free **and** the `cts_n` input is low. CTS is only checked at the start of each byte: a byte in progress is always finished.
- In `top.v` the CTS input is tied to active (0) unless it is driven from outside.

### Reception
- A falling edge on `rx` starts reception. In the middle of the start bit the line is checked to still be 0 (otherwise it is a glitch and is ignored); after that it is sampled every `BAUD_DIV` clocks.
- **Stop = 1:** the byte is pushed into the RX FIFO. If the FIFO was full, the byte is dropped and `OVERRUN` is set; the oldest bytes are kept.
- **Stop = 0 (framing error):** the byte is dropped, `FRAME_ERR` is set, and the receiver waits for the line to return to 1 before looking for the next start bit.

### Reset
FIFOs empty, flags at 0, `RTS = 0` (the other end should not send until the firmware enables it) and `BAUD_DIV = UART1_SIM_BAUD_DIV`.

## 5. Flow control (RTS/CTS)

The RX FIFO holds only 16 bytes. If the firmware is slow to read it and the other end keeps sending, the FIFO fills up and the following bytes are lost (`OVERRUN`). Flow control lets the firmware stop the other end:

- The firmware writes `CTRL.RTS = 0` to ask it to stop and `CTRL.RTS = 1` when there is room again. The `rts_n` line is the negation of that bit.
- The other end usually looks at `rts_n` only at the start of each byte: the byte in progress is always finished, and a sender with latency may send a few more bytes after seeing RTS drop. The UART does **not** lower RTS by itself: the firmware decides when.

**Budget.** If RTS is lowered when the FIFO holds `L` bytes, the bytes the sender already had in flight can still arrive (`K`: the byte in progress plus any it sends because of latency). If the firmware takes `T` clocks between one decision and the next, another `T / (10 × BAUD_DIV)` bytes arrive in that interval. To avoid losing bytes:

```
L + (bytes arriving between two decisions) + K  ≤  UART1_FIFO_DEPTH
```

The threshold to raise RTS again must be lower than the one used to lower it (hysteresis), so that RTS does not toggle on every byte.
