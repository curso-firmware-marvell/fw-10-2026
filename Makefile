TOP = testbench
FILELIST = filelist.f
OBJ_DIR = obj_dir
WAVE = wave.vcd

VERILATOR = verilator
# VER_FLAGS = --binary --trace -Wall -f $(FILELIST)
VER_FLAGS = --binary --trace -Wno-fatal -f $(FILELIST)
# VER_FLAGS = --binary -DEBUG_PRINTS --trace -Wno-fatal -f $(FILELIST) 
# VER_FLAGS = --binary --trace --x-assign unique --x-initial unique -Wno-fatal -f $(FILELIST)

FW_DIR = fw
FW_HEX = $(FW_DIR)/fw.hex

.PHONY: all fw sim wave clean clean_sim build_project_config

all: build_project_config fw clean_sim sim wave

build_project_config:
	python3 build_project_config.py

fw:
	$(MAKE) -C $(FW_DIR)

sim: $(FW_HEX)
	$(VERILATOR) $(VER_FLAGS)
	./$(OBJ_DIR)/V$(TOP)

wave:
	gtkwave $(WAVE)

clean:
	rm -rf $(OBJ_DIR) *.vcd *.fst
	$(MAKE) -C $(FW_DIR) clean

clean_sim:
	rm -rf $(OBJ_DIR) *.vcd *.fst