# Toolchain
CC      = /opt/riscv/bin/riscv64-unknown-elf-gcc
OBJDUMP = /opt/riscv/bin/riscv64-unknown-elf-objdump
OBJCOPY = /opt/riscv/bin/riscv64-unknown-elf-objcopy

# Files
ELF = current.elf
BIN = $(ELF).bin
HEX = fw.hex

# Flags 
CFLAGS = -march=rv32i -mabi=ilp32 -O2 -ffreestanding -nostdlib -nostartfiles



LDFLAGS = -Tlinker.ld -Wl,--gc-sections -Wl,-verbose
# LDFLAGS = -T linker.ld -Wl,--gc-sections -lgcc ## adding floating point support

# SRC = startup.S print.c test_adc.c
# SRC = startup.S print.c test_dma.c 
SRC = startup.S print.c test_fw_status.c
# SRC = startup.S print.c test_new_uart.c

.PHONY: all disasm clean 

all: $(HEX)

# Build ELF
$(ELF): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -lgcc
	$(OBJDUMP) -d $(ELF) 

# Disassembly
disasm: $(ELF)
	$(OBJDUMP) -d $(ELF)

# ELF -> BIN
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@
	
# BIN -> HEX (one instruction per line)
$(HEX): $(BIN)
	hexdump -v -e '1/4 "%08x\n"' $< > $@
	
# Clean
clean:
	rm -f $(ELF) $(BIN) $(HEX)
	