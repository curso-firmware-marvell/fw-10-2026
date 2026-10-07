# Datasheet

**Scope:** This document describes the SoC implemented in `rtl/top.v` for the
*Curso de Firmware* platform: a PicoRV32 (RISC-V RV32I) core with a small bus
fabric, a register file, a DMA controller, a simulated ADC, two UARTs, and a
simple GPIO/LED register. It is meant to be the reference students use to
write **firmware** (C, using `fw/config.h`) and to reason about the
**hardware behavior** behind each register (from the RTL in `rtl/`).

**Usage model:** this platform is used in **simulation only**, with
Verilator. `rtl/testbench.sv` instantiates `top.v` directly (no FPGA wrapper,
no synthesis, no real I/O pins involved). Firmware is built with
`fw/Makefile` into `fw/fw.hex`, which the testbench's `init_memory()` task
loads straight into instruction memory before releasing reset.

**`project_config.def`** is processed
by `build_project_config.py` (invoked via `make build_project_config`) to
generate three files that must always agree with each other:
`fw/config.h` (C macros), `rtl/config.vh` (Verilog defines/parameters) and
`rtl/register_file.v` (the register file module itself). **Never hand-edit
those three generated files**, just edit `project_config.def` and re-run the
generator. 

---

## 1. System overview

![Alt text](media/mcu.png)

* **CPU:** [PicoRV32](https://github.com/YosysHQ/picorv32) (`submodules/picorv32`), RV32I base ISA. All parameters in `top.v` are left at their instantiation defaults (empty parens), so compressed ISA, multiply/divide, IRQs, etc. are disabled unless explicitly set. `PROGADDR_RESET` defaults to `0x0000_0000` (matches `ADDR_START_INST`). `STACKADDR` is a core default only — the actual stack pointer is set by firmware's `startup.S`/`linker.ld` (see §6).
* **Bus:** single 32-bit address / 32-bit data, byte-addressed, with a 4-bit write-strobe (`wstrb`) for per-byte writes — the standard picorv32 native memory interface.
* **Two bus masters:** the CPU (master 0, instructions + data) and the DMA controller (master 1), combined by `mem_arbitrator` before reaching the single address decoder `mem_mux`.
* **Three address regions** decoded by `mem_mux`: instruction memory, data memory, and the register file (peripherals).

---

## 2. Global memory map

| Region | Base | Size | End (incl.) | Backing store | R/W |
|---|---|---|---|---|---|
| Instruction memory | `0x0000_0000` | 128 KiB (`0x0002_0000`) | `0x0001_FFFF` | `mem_mux.memory_i` | R/W |
| Data memory (RAM)   | `0x0002_0000` | 128 KiB (`0x0002_0000`) | `0x0003_FFFF` | `mem_mux.memory_d` | R/W |
| Register file / peripherals | `0x0004_0000` | 128 KiB (`0x0002_0000`) | `0x0005_FFFF` | `register_file.v` | see §4 |
| *(unmapped)* | `0x0006_0000` | — | `0xFFFF_FFFF` | none | — |

Instruction memory is writable over the bus (per-byte `wstrb`), including by the DMA controller, so self-modifying code and DMA-into-code-memory both work (exercised by `fw/test_dma.c`). An access entirely outside `0x0000_0000`–`0x0005_FFFF` never asserts `mem_ready`, so the CPU stalls forever instead of taking a fault — avoid wild pointers.

---

## 3. Bus arbitration (`mem_arbitrator.v`)

* The CPU (master 0) always has priority. The DMA (master 1) only gets the bus when `i_master_0_mem_valid` is low.
* It is a 2-state FSM (`MASTER_0_REQ` / `MASTER_1_REQ`) that only switches ownership once the previous owner's current transaction has completed (`slave_ready_done`), so a transfer in flight is never interrupted mid-cycle.


---

## 4. Register file (`register_file.v`) — detailed reference

All registers are 32-bit, memory-mapped, accessed with ordinary 32-bit loads/stores (`volatile unsigned int`). Reads and writes complete combinationally/in a single bus cycle — there are no wait states.

Note: the read-decode logic is a chain of `if` statements with no default case. Any address inside the register-file window that doesn't match a register listed below simply holds whatever was last read (inferred latch) — reserved addresses do **not** reliably read as zero, so don't rely on them.

### 4.1 Status / scratch registers

| Register | Addr | Access | Description |
|---|---|---|---|
| `REG_FW_STATUS`  | `0x4_0000` | RW | Firmware status flag. Convention used by all test firmware: `0x00` = running, `0xFF` = PASS, `0xBAD` = FAIL. `testbench.sv` ends the simulation when this becomes `0xFF` (or the core traps) — follow the same convention in your own firmware. |
| `REG_FW_REPORT`  | `0x4_0004` | RW | Free-form scratch register for progress counters or debug values. Pure software convention, no hardware behavior. |
| `REG_AUX_0`      | `0x4_0010` | RW | General-purpose scratch register. Also used as a cheap busy-wait counter in some test firmware. In the testbench, writing the exact value `0x5ED5ED` is a simulation-only backdoor that triggers a canned byte sequence into UART0's RX path — has no effect beyond this repo's testbench. |
| `REG_MCU_RESET`  | `0x4_0014` | RW | Core reset control, bit 0 only. Writing `1` holds the CPU in reset (`resetn & ~mcu_reset_rf`); since the CPU can't execute while held in reset, it cannot clear this register itself. Only an external reset or another bus master can release it. Not used by the current simulation flow — `testbench.sv` drives reset directly. |

### 4.2 DMA controller (`dma.v`)

| Register | Addr | Access | Description |
|---|---|---|---|
| `REG_DMA_SOURCE_ADDR` | `0x4_0018` | RW | Source byte address. Latched on the start edge, so it only needs to be valid at trigger time. |
| `REG_DMA_DEST_ADDR`   | `0x4_001C` | RW | Destination byte address. Same latch-on-start behavior. |
| `REG_DMA_SIZE`        | `0x4_0020` | RW | Transfer length in 32-bit words. 
| `REG_DMA_START`       | `0x4_0024` | RW | Start trigger, edge-sensitive. Always write `0` then `1` to guarantee a rising edge, even if you believe it already holds `0`. |
| `REG_DMA_DONE`        | `0x4_0028` | RO | `1` once the transfer completes; cleared on the next start edge. |

The DMA is bus master 1, a generic 32-bit-word memory-to-memory copier that can move data between any combination of instruction memory, data memory, and the register file (including array registers such as the UART buffers or the ADC sample buffer). One word per READ_REQ/WRITE_REQ cycle pair.

### 4.3 ADC (`adc.sv`)

| Register | Addr | Access | Description |
|---|---|---|---|
| `REG_ADC_START`       | `0x4_002C` | RW | Bit 0: edge-triggered start of one 256-sample acquisition. Bit 1: circular mode  |
| `REG_ADC_STATUS`      | `0x4_0030` | RO | Bit 0 (`half_done`): set after 128 samples. Bit 1 (`acq_done`): set after all 256 samples. Both clear on the next start edge. |
| `REG_ADC_CLK_DIVIDER` | `0x4_0034` | RW | Sample-rate divider in clock cycles per sample. Must be set before the start edge. |
| `REG_ARRAY_ADC_BUFF`  | `0x4_0100`–`0x4_04FF` (256 words) | RO array | Captured samples, one 32-bit code per sample, in acquisition order. |

The ADC is a behavioral simulation model, not synthesizable hardware: it generates a 1 MHz sine wave internally (`$sin`/`$realtime`) and quantizes it to a 32-bit code over `0..VREF` (3.3 V). It exists to exercise the register/DMA/firmware interface of an ADC peripheral; only the register-level contract (start/status/divider/buffer) is meant to be realistic.

### 4.4 LED / GPIO register

| Register | Addr | Access | Description |
|---|---|---|---|
| `REG_LED` | `0x4_0038` | RW | General-purpose output register. Only the low 8 bits are meaningful. `testbench.sv` prints them to the console on every change (`LED: 0bxxxxxxxx`). Not connected to any physical pin in this repo; it is a simulation-visible exercise register, not real hardware I/O. |

### 4.5 UART0 / UART1 (`uart.v`, two identical instances)

Two independent, identically-laid-out peripherals. Addresses below are for UART0 (base `0x4_2000`); UART1 is the same layout at base `0x4_3000` (offset `+0x1000`).

| Register | UART0 Addr | UART1 Addr | Access | Description |
|---|---|---|---|---|
| `..._START_TRANSMIT`     | `0x4_2000` | `0x4_3000` | RW | Write `1` to begin transmitting `..._SIZE_TO_TRANSMIT` bytes from `..._BUFF_OUT`. The start condition is level-sensitive, so leaving it at `1` after a transfer finishes will retrigger the next idle cycle. Always clear it back to `0` right after setting it. |
| `..._SIZE_TO_TRANSMIT`   | `0x4_2008` | `0x4_3008` | RW | Number of words to transmit from `..._BUFF_OUT`, starting at index 0 (max 256). |
| `..._TRANSMISION_DONE`   | `0x4_200C` | `0x4_300C` | RO | `1` when idle/complete (also `1` at reset); `0` while transmitting. |
| `..._BUFF_OUT` (array)   | `0x4_2010`–`0x4_240F` | `0x4_3010`–`0x4_340F` | RW array, 256 words | Output buffer. |
| `..._DATA_AVAILABLE`     | `0x4_2800` | `0x4_3800` | RO | `1` once at least one byte has been received since the last flush. |
| `..._DATA_FLUSH`         | `0x4_2804` | `0x4_3804` | RW | Set to `1` to reset the RX byte counter to `0`, discarding buffered input. |
| `..._INDEX_IN`           | `0x4_2808` | `0x4_3808` | RO | Number of bytes currently held in `..._BUFF_IN`. |
| `..._BUFF_IN` (array)    | `0x4_280C`–`0x4_2C0B` | `0x4_380C`–`0x4_3C0B` | RO array, 256 words | Received buffer. |


---


## 6. Firmware programming model

* Include `includes.h` (which pulls in the generated `config.h`), giving every register above as a ready-to-use C macro/pointer, e.g.:
  ```c
  REG_FW_STATUS = 0x0;
  REG_ARRAY_UART0_BUFF_OUT[0] = 'h';
  while (!REG_UART0_TRANSMISION_DONE) {}
  ```
* Registers are exposed via `(*(volatile unsigned int*)addr)`, and array registers via `((volatile unsigned int*)base)[i]` — `volatile` is essential since these map to hardware, not RAM.
* `fw/print.c` is a small driver built on UART0 (`print_str`, `print_chr`, `print_dec`, `print_hex`) — a useful reference for wrapping a register interface into an ergonomic C API, and a reasonable base for a UART1 driver.
* `startup.S` sets the stack pointer from `_stack_top` (defined in `linker.ld` as the top of data RAM, i.e. the first byte below the register file) and jumps to `main()`. There is no interrupt vector table wired up — don't rely on interrupts unless you add that plumbing yourself.
* Build with `fw/Makefile` (`riscv64-unknown-elf-gcc -march=rv32i -mabi=ilp32`, no standard library). It produces `fw.hex`, one 32-bit word per line in hex. This is the file `testbench.sv`'s `init_memory()` task loads into instruction memory for simulation — this is the only loading path used in this course.
