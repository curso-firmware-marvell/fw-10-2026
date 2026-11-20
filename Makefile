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
RUN_TEST = bash $(FW_DIR)/tools/run_test.sh

# Tests: un make test_<nombre> por cada fw/tests/test_<nombre>.c, y make test_main para app/main.c
TESTS = $(basename $(notdir $(wildcard $(FW_DIR)/tests/test_*.c))) test_main
target_of = $(if $(filter test_main,$(1)),main,$(1))
# variables opcionales que llegan a run_test.sh (MAX_CYCLES, UART1_SCRIPT=<patron>, VERBOSE=1)
export MAX_CYCLES UART1_SCRIPT VERBOSE

.PHONY: all fw sim wave clean clean_sim build_project_config simbin run test $(TESTS)

all: build_project_config fw clean_sim sim wave

build_project_config:
	python3 build_project_config.py

fw:
	$(MAKE) -C $(FW_DIR)

# La salida de la simulacion se guarda en $(SIM_LOG) (la leen fw/tools/plot_adc.py y make -C fw check)
SIM_LOG ?= $(FW_DIR)/build/sim.log

sim: $(FW_HEX)
	$(VERILATOR) $(VER_FLAGS)
	@mkdir -p $(dir $(SIM_LOG))
	stdbuf -oL ./$(OBJ_DIR)/V$(TOP) 2>&1 | tee $(SIM_LOG)

wave:
	gtkwave $(WAVE)

# ---- tests --------------------------------------------------------------------------------
# make run            programa por defecto (fw/app/main.c) en la simulacion, con wave.vcd
# make test_<nombre>  un test (ej. make test_adc); opcionales: STANDALONE=1, CORRUPT=1
# make test_main      app/main.c contra el patron fw/tests/main.txt (UART1_SCRIPT=otro, VERBOSE=1)
# make test           todos los tests con bootloader + 2 pruebas negativas; falla si alguno falla
# Los tests con un patron fw/tests/<TARGET>.txt (UART1) lo usan solos como host de UART1.

# Compila el simulador (sin tocar fw/fw.hex)
simbin: build_project_config
	@echo "verilator: compilando el simulador (log en verilator.log)"
	@$(VERILATOR) $(VER_FLAGS) > verilator.log 2>&1 || { tail -30 verilator.log; exit 1; }

run: build_project_config fw sim

$(TESTS): %: simbin
	@$(RUN_TEST) $@ PASS TARGET=$(call target_of,$@) $(if $(STANDALONE),STANDALONE=$(STANDALONE)) $(if $(CORRUPT),CORRUPT=$(CORRUPT))

test: simbin
	@fails=0; export NOWAVE=1; \
	for t in $(TESTS); do tg=$$t; [ "$$t" = test_main ] && tg=main; \
	    $(RUN_TEST) $$t PASS TARGET=$$tg || fails=$$((fails+1)); done; \
	echo "-- pruebas negativas (deben fallar a proposito) --"; \
	$(RUN_TEST) test_data_bss_standalone FAIL TARGET=test_data_bss STANDALONE=1 || fails=$$((fails+1)); \
	$(RUN_TEST) test_fw_status_corrupt FAIL TARGET=test_fw_status CORRUPT=1 || fails=$$((fails+1)); \
	if [ $$fails -eq 0 ]; then echo "TODOS LOS TESTS OK"; else echo "$$fails TEST(S) FALLARON"; exit 1; fi

clean:
	rm -rf $(OBJ_DIR) *.vcd *.fst
	$(MAKE) -C $(FW_DIR) clean

clean_sim:
	rm -rf $(OBJ_DIR) *.vcd *.fst
