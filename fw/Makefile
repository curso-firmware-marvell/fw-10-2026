# =============================================================================
#  TP4 — BOOTLOADER + FW del TP anterior en UNA imagen
#
#  Este Makefile YA ESTÁ COMPLETO: lo que tienen que escribir son los linker
#  scripts y el código del BL y del arranque del FW (buscar "TODO"). Igual
#  léanlo: en el hito 9 se pregunta qué hace cada paso.
# =============================================================================
#   make             build/bl.elf + build/app.elf -> build/fw.hex -> ../fw/fw.hex
#   make sim         lo anterior + simulación de la plataforma desde ../
#   make check       diapo 12: fw.hex vs lo que cargó el simulador (después de make sim)
#   make inspect     size / readelf / nm / vector tables decodificadas
#   make disasm      build/bl.dis y build/app.dis
#   make clean
#
#   APP_SRC="print.c test_fw_status.c"  FW anterior; se busca en ../fw/app, ../fw/tests y ../fw/lib/src
#   APP_MARCH=rv32i                     -march del FW (el del TP2: rv32ic, rv32imc... si top.v lo soporta)
#   CORRUPT=1                           prueba negativa: 1 bit del FW invertido después de firmarlo
#   STANDALONE=1                        flujo original del FW, sin BL, para comparar
#   FW_LIB=...                          carpeta local con src/strutils.c e inc/strutils.h
#   PLATFORM=...                        raíz de la plataforma (default ../)
#
#  Correr desde esta carpeta (o con make -C). El flujo integrado instala fw.hex en
#  ../fw/, que es la imagen que carga el testbench de la plataforma.
# =============================================================================

PLATFORM ?= ../
FW_DIR   ?= $(PLATFORM)/fw
# Biblioteca local de CRC; se puede reemplazar con FW_LIB=...
FW_LIB   ?= $(FW_DIR)/lib
TOOLS     = $(FW_DIR)/tools
BUILD     = build

# ---------------- Toolchain: /opt/riscv o el disponible en PATH ----------------
RISCV_BIN ?= $(if $(shell command -v riscv64-unknown-elf-gcc 2>/dev/null),,/opt/riscv/bin/)
CROSS     ?= $(RISCV_BIN)riscv64-unknown-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
READELF = $(CROSS)readelf
NM      = $(CROSS)nm
SIZE    = $(CROSS)size
PYTHON ?= python3

# Flags de firmware (+ -g, que no cambia el código y sirve para gdb/objdump -S)
# El BL va siempre en rv32i: tiene que arrancar con CUALQUIER configuración de top.v.
# El FW puede usar el -march del TP2 (rv32ic, rv32imc...) si top.v lo soporta.
BL_MARCH   = rv32i
APP_MARCH ?= rv32i
COMMON     = -mabi=ilp32 -O2 -ffreestanding -nostdlib -nostartfiles -g -I$(FW_DIR) -I$(FW_LIB)/inc
# BL: una sección por función (--gc-sections tira lo que no usa de print.c), warnings,
# y que GCC no convierta loops en llamadas a memcpy/memset (no hay libc)
BL_CFLAGS  = -march=$(BL_MARCH) $(COMMON) -ffunction-sections -fdata-sections -Wall -fno-tree-loop-distribute-patterns
APP_CFLAGS = -march=$(APP_MARCH) $(COMMON)
LDFLAGS    = -Wl,--gc-sections
LDLIBS     = -lgcc

# ---------------- Fuentes ----------------
APP_SRC ?= print.c test_fw_status.c
src_path  = $(firstword $(wildcard $(1) $(FW_DIR)/$(1) $(FW_DIR)/app/$(notdir $(1)) $(FW_DIR)/tests/$(notdir $(1)) $(FW_LIB)/src/$(notdir $(1))) $(1))
APP_FILES = $(foreach f,$(APP_SRC),$(call src_path,$(f)))
# El BL usa print.c y la implementación CRC local (--gc-sections deja solo lo que usa)
BL_SRC    = $(FW_DIR)/bootloader/bl_start.S $(FW_DIR)/bootloader/bl_main.c $(FW_LIB)/src/print.c $(FW_LIB)/src/strutils.c
COMMON_DEPS = $(FW_DIR)/memory_map.ld $(FW_DIR)/fw_image.h $(FW_LIB)/inc/includes.h $(FW_DIR)/config.h $(FW_LIB)/inc/strutils.h

.PHONY: all install sim check inspect disasm clean FORCE
.DELETE_ON_ERROR:

all: $(BUILD)/fw.hex $(BUILD)/bl.sym $(BUILD)/app.sym install

$(BUILD):
	@mkdir -p $@

# Guarda con qué opciones se armó la imagen: si cambian (otro APP_SRC, CORRUPT,
# STANDALONE...), se rearma lo necesario aunque los archivos no hayan cambiado.
MODE = APP_SRC=$(APP_SRC) APP_MARCH=$(APP_MARCH) CORRUPT=$(CORRUPT) STANDALONE=$(STANDALONE)
$(BUILD)/fw.mode: FORCE | $(BUILD)
	@echo '$(MODE)' | cmp -s - $@ || echo '$(MODE)' > $@

$(FW_DIR)/config.h:
	$(error Falta $(FW_DIR)/config.h: corré "make build_project_config" desde la raíz del repositorio)

# ---------------- Bootloader -> BL_ROM (0x0000_0000) ----------------
$(BUILD)/bl.elf: $(BL_SRC) $(FW_DIR)/bootloader/bl.ld $(COMMON_DEPS) | $(BUILD)
	$(CC) $(BL_CFLAGS) $(LDFLAGS) -T $(FW_DIR)/bootloader/bl.ld -Wl,-Map=$(BUILD)/bl.map $(BL_SRC) -o $@ $(LDLIBS)
	$(SIZE) $@

# ---------------- FW del TP anterior -> APP_ROM (0x0000_1000) ----------------
$(BUILD)/app.elf: $(FW_DIR)/app/app_startup.S $(APP_FILES) $(FW_DIR)/app/app.ld $(COMMON_DEPS) $(BUILD)/fw.mode
	$(CC) $(APP_CFLAGS) $(LDFLAGS) -T $(FW_DIR)/app/app.ld -Wl,-Map=$(BUILD)/app.map $(FW_DIR)/app/app_startup.S $(APP_FILES) -o $@ $(LDLIBS)
	$(SIZE) $@

# ---------------- Flujo standalone anterior (sin BL), para comparar ----------------
$(BUILD)/standalone.elf: $(FW_DIR)/startup.S $(APP_FILES) $(FW_DIR)/linker.ld $(FW_DIR)/config.h $(BUILD)/fw.mode
	$(CC) $(APP_CFLAGS) $(LDFLAGS) -T $(FW_DIR)/linker.ld -Wl,-Map=$(BUILD)/standalone.map $(FW_DIR)/startup.S $(APP_FILES) -o $@ $(LDLIBS)
	$(SIZE) $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/%.sym: $(BUILD)/%.elf
	$(NM) -n $< > $@

# ---------------- Imagen que carga el testbench ----------------
ifeq ($(STANDALONE),1)
$(BUILD)/fw.bin: $(BUILD)/standalone.bin $(BUILD)/fw.mode
	cp $< $@
else
$(BUILD)/fw.bin: $(BUILD)/bl.bin $(BUILD)/app.bin $(TOOLS)/mkimage.py $(BUILD)/fw.mode
	$(PYTHON) $(TOOLS)/mkimage.py --bl $(BUILD)/bl.bin --app $(BUILD)/app.bin -o $@ $(if $(filter 1,$(CORRUPT)),--corrupt)
endif

# Formato de imagen del testbench: una palabra de 32 bits por línea
$(BUILD)/fw.hex: $(BUILD)/fw.bin
	hexdump -v -e '1/4 "%08x\n"' $< > $@

# init_memory() de rtl/testbench.sv lee fw/fw.hex desde la raíz del repositorio
install: $(BUILD)/fw.hex
	cp $< $(FW_DIR)/fw.hex
	@echo "-> $(FW_DIR)/fw.hex actualizado ($(if $(filter 1,$(STANDALONE)),FW solo sin BL,BL + FW))"

# ---------------- Simulación y verificación ----------------
sim: all
	cd $(PLATFORM) && $(MAKE) sim 2>&1 | tee $(CURDIR)/$(BUILD)/sim.log

check:
	$(PYTHON) $(TOOLS)/check_load.py --hex $(BUILD)/fw.hex --log $(BUILD)/sim.log --sym $(BUILD)/bl.sym --sym $(BUILD)/app.sym

KEY_SYMS = _bl_vectors|bl_reset_handler|bl_main|bl_jump|__fw_slot_start|__fw_header|Reset_Handler|main|_sidata|_sdata|_edata|_sbss|_ebss|_stack_top

inspect: $(BUILD)/bl.elf $(BUILD)/app.elf $(BUILD)/bl.sym $(BUILD)/app.sym $(BUILD)/fw.hex
	@echo "===== (1) Secciones y tamaños [size -A] ====="
	@$(SIZE) -A -x $(BUILD)/bl.elf $(BUILD)/app.elf
	@echo "===== (2) FW: .data con VMA en RAM y LMA dentro de la imagen [readelf -S / -l] ====="
	@$(READELF) -S -W $(BUILD)/app.elf
	@$(READELF) -l -W $(BUILD)/app.elf
	@echo "===== (3) Entry points [readelf -h] ====="
	@printf "bl.elf : "; $(READELF) -h $(BUILD)/bl.elf  | grep Entry
	@printf "app.elf: "; $(READELF) -h $(BUILD)/app.elf | grep Entry
	@echo "===== (4) Símbolos clave [nm -n] ====="
	@cd $(BUILD) && grep -E ' ($(KEY_SYMS))$$' bl.sym app.sym
	@echo "===== (5) Vector tables y header leídos de fw.hex [tools/check_load.py] ====="
	@$(PYTHON) $(TOOLS)/check_load.py --hex $(BUILD)/fw.hex --sym $(BUILD)/bl.sym --sym $(BUILD)/app.sym

disasm: $(BUILD)/bl.elf $(BUILD)/app.elf
	$(OBJDUMP) -d -S $(BUILD)/bl.elf  > $(BUILD)/bl.dis
	$(OBJDUMP) -d -S $(BUILD)/app.elf > $(BUILD)/app.dis
	@echo "-> $(BUILD)/bl.dis  $(BUILD)/app.dis"

clean:
	rm -rf $(BUILD)

FORCE:
