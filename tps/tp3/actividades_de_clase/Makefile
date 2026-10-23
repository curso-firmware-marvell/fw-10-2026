CC        = gcc
CXX       = g++
AS        = as
AR        = ar
RANLIB    = ranlib

BUILD_DIR = build

# --- Directory layout ---

APP_DIR   = src/app
LIB_DIR   = src/lib
HAL_DIR   = src/hal
DEMO_DIR  = src/demo
CPP_DIR   = src/cpp
ASM_DIR   = src/asm

INCLUDES  = -I$(LIB_DIR) -I$(HAL_DIR) -I$(DEMO_DIR) -I$(CPP_DIR) -I$(APP_DIR)

CFLAGS    = -Wall -Wextra -std=c11 -g -O0 $(INCLUDES)
CFLAGS   += -ffunction-sections -fdata-sections
CXXFLAGS  = -Wall -Wextra -std=c++17 -g -O0 $(INCLUDES)
CXXFLAGS += -ffunction-sections -fdata-sections
LDFLAGS   = -Wl,--gc-sections

# --- Source files ---

MAIN_SRC       = $(APP_DIR)/main.c

MATHLIB_SRC    = $(LIB_DIR)/mathlib.c
STRUTILS_SRC   = $(LIB_DIR)/strutils.c

HAL_UART_SRC   = $(HAL_DIR)/hal_uart.c
HAL_RING_SRC   = $(HAL_DIR)/hal_ring.c
HAL_SYSTEM_SRC = $(HAL_DIR)/hal_system.c

DEMO_MATH_SRC     = $(DEMO_DIR)/demo_math.c
DEMO_STR_SRC      = $(DEMO_DIR)/demo_str.c
DEMO_EMBEDDED_SRC = $(DEMO_DIR)/demo_embedded.c

CPPWRAP_SRC    = $(CPP_DIR)/cpp_wrapper.cpp
ASM_SRC        = $(ASM_DIR)/asm_example.s

ALL_SRCS = $(wildcard $(APP_DIR)/*.c $(LIB_DIR)/*.[ch] $(HAL_DIR)/*.[ch] $(DEMO_DIR)/*.[ch] $(CPP_DIR)/*)

# --- Object files (all go into build/) ---

MAIN_OBJ       = $(BUILD_DIR)/main.o
MATHLIB_OBJ    = $(BUILD_DIR)/mathlib.o
STRUTILS_OBJ   = $(BUILD_DIR)/strutils.o
CPPWRAP_OBJ    = $(BUILD_DIR)/cpp_wrapper.o
ASM_OBJ        = $(BUILD_DIR)/asm_example.o

HAL_UART_OBJ   = $(BUILD_DIR)/hal_uart.o
HAL_RING_OBJ   = $(BUILD_DIR)/hal_ring.o
HAL_SYSTEM_OBJ = $(BUILD_DIR)/hal_system.o

DEMO_MATH_OBJ     = $(BUILD_DIR)/demo_math.o
DEMO_STR_OBJ      = $(BUILD_DIR)/demo_str.o
DEMO_EMBEDDED_OBJ = $(BUILD_DIR)/demo_embedded.o

# --- Aggregate lists ---

C_OBJS  = $(MAIN_OBJ) $(MATHLIB_OBJ) $(STRUTILS_OBJ) \
          $(HAL_UART_OBJ) $(HAL_RING_OBJ) $(HAL_SYSTEM_OBJ) \
          $(DEMO_MATH_OBJ) $(DEMO_STR_OBJ) $(DEMO_EMBEDDED_OBJ)

LIB_OBJS = $(MATHLIB_OBJ) $(STRUTILS_OBJ) \
           $(HAL_UART_OBJ) $(HAL_RING_OBJ) $(HAL_SYSTEM_OBJ)

ALL_OBJS = $(C_OBJS) $(CPPWRAP_OBJ) $(ASM_OBJ)

STATIC_LIB  = $(BUILD_DIR)/libutils.a
TARGET      = $(BUILD_DIR)/binutils_demo
TARGET_DBG  = $(BUILD_DIR)/binutils_demo_debug
TARGET_OPT  = $(BUILD_DIR)/binutils_demo_opt
TARGET_STRIP= $(BUILD_DIR)/binutils_demo_stripped
TARGET_NOGC = $(BUILD_DIR)/binutils_demo_nogc
TARGET_LIB  = $(BUILD_DIR)/binutils_demo_lib

APP_OBJS = $(MAIN_OBJ) $(DEMO_MATH_OBJ) $(DEMO_STR_OBJ) $(DEMO_EMBEDDED_OBJ) $(ASM_OBJ) $(CPPWRAP_OBJ)

.PHONY: all clean directories

all: directories $(TARGET) $(STATIC_LIB) $(ASM_OBJ) $(CPPWRAP_OBJ) $(TARGET_DBG) $(TARGET_OPT) $(TARGET_STRIP) $(TARGET_NOGC) $(TARGET_LIB)

directories:
	@mkdir -p $(BUILD_DIR)

# --- Object files (C) ---

$(MAIN_OBJ): $(MAIN_SRC) $(DEMO_DIR)/demo.h
	$(CC) $(CFLAGS) -c $< -o $@

$(MATHLIB_OBJ): $(MATHLIB_SRC) $(LIB_DIR)/mathlib.h
	$(CC) $(CFLAGS) -c $< -o $@

$(STRUTILS_OBJ): $(STRUTILS_SRC) $(LIB_DIR)/strutils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(HAL_UART_OBJ): $(HAL_UART_SRC) $(HAL_DIR)/hal_uart.h $(LIB_DIR)/mathlib.h
	$(CC) $(CFLAGS) -c $< -o $@

$(HAL_RING_OBJ): $(HAL_RING_SRC) $(HAL_DIR)/hal_ring.h
	$(CC) $(CFLAGS) -c $< -o $@

$(HAL_SYSTEM_OBJ): $(HAL_SYSTEM_SRC) $(HAL_DIR)/hal_system.h $(LIB_DIR)/mathlib.h
	$(CC) $(CFLAGS) -c $< -o $@

$(DEMO_MATH_OBJ): $(DEMO_MATH_SRC) $(DEMO_DIR)/demo.h $(LIB_DIR)/mathlib.h
	$(CC) $(CFLAGS) -c $< -o $@

$(DEMO_STR_OBJ): $(DEMO_STR_SRC) $(DEMO_DIR)/demo.h $(LIB_DIR)/strutils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(DEMO_EMBEDDED_OBJ): $(DEMO_EMBEDDED_SRC) $(DEMO_DIR)/demo.h $(HAL_DIR)/hal_uart.h $(HAL_DIR)/hal_ring.h $(HAL_DIR)/hal_system.h $(LIB_DIR)/mathlib.h
	$(CC) $(CFLAGS) -c $< -o $@

# --- Object files (C++) ---

$(CPPWRAP_OBJ): $(CPPWRAP_SRC) $(CPP_DIR)/cpp_wrapper.h $(LIB_DIR)/mathlib.h $(LIB_DIR)/strutils.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Object files (Assembly) ---

$(ASM_OBJ): $(ASM_SRC)
	$(AS) $< -o $@

# --- Static library (C modules) ---

$(STATIC_LIB): $(LIB_OBJS)
	$(AR) rcs $@ $^
	$(RANLIB) $@

# --- Default executable (link with g++ because of C++ object) ---

$(TARGET): $(C_OBJS) $(ASM_OBJ) $(CPPWRAP_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

# Mismos objetos, sin --gc-sections: para comparar tamaños con size
$(TARGET_NOGC): $(C_OBJS) $(ASM_OBJ) $(CPPWRAP_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Aplicación y demos enlazadas contra la biblioteca estática
$(TARGET_LIB): $(APP_OBJS) $(STATIC_LIB)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(APP_OBJS) -L$(BUILD_DIR) -lutils -o $@

# --- Debug and optimised variants ---

DBG_C_FLAGS   = -Wall -std=c11 -g3 -O0 -DDEBUG $(INCLUDES)
DBG_CXX_FLAGS = -Wall -std=c++17 -g3 -O0 -DDEBUG $(INCLUDES)
OPT_C_FLAGS   = -Wall -std=c11 -O2 -DNDEBUG -ffunction-sections -fdata-sections $(INCLUDES)
OPT_CXX_FLAGS = -Wall -std=c++17 -O2 -DNDEBUG -ffunction-sections -fdata-sections $(INCLUDES)

DBG_C_OBJS = $(BUILD_DIR)/main_dbg.o $(BUILD_DIR)/mathlib_dbg.o $(BUILD_DIR)/strutils_dbg.o \
             $(BUILD_DIR)/hal_uart_dbg.o $(BUILD_DIR)/hal_ring_dbg.o $(BUILD_DIR)/hal_system_dbg.o \
             $(BUILD_DIR)/demo_math_dbg.o $(BUILD_DIR)/demo_str_dbg.o $(BUILD_DIR)/demo_embedded_dbg.o

OPT_C_OBJS = $(BUILD_DIR)/main_opt.o $(BUILD_DIR)/mathlib_opt.o $(BUILD_DIR)/strutils_opt.o \
             $(BUILD_DIR)/hal_uart_opt.o $(BUILD_DIR)/hal_ring_opt.o $(BUILD_DIR)/hal_system_opt.o \
             $(BUILD_DIR)/demo_math_opt.o $(BUILD_DIR)/demo_str_opt.o $(BUILD_DIR)/demo_embedded_opt.o

$(TARGET_DBG): $(ALL_SRCS) $(ASM_OBJ)
	$(CC)  $(DBG_C_FLAGS) -c $(MAIN_SRC)          -o $(BUILD_DIR)/main_dbg.o
	$(CC)  $(DBG_C_FLAGS) -c $(MATHLIB_SRC)        -o $(BUILD_DIR)/mathlib_dbg.o
	$(CC)  $(DBG_C_FLAGS) -c $(STRUTILS_SRC)       -o $(BUILD_DIR)/strutils_dbg.o
	$(CC)  $(DBG_C_FLAGS) -c $(HAL_UART_SRC)       -o $(BUILD_DIR)/hal_uart_dbg.o
	$(CC)  $(DBG_C_FLAGS) -c $(HAL_RING_SRC)       -o $(BUILD_DIR)/hal_ring_dbg.o
	$(CC)  $(DBG_C_FLAGS) -c $(HAL_SYSTEM_SRC)     -o $(BUILD_DIR)/hal_system_dbg.o
	$(CC)  $(DBG_C_FLAGS) -c $(DEMO_MATH_SRC)      -o $(BUILD_DIR)/demo_math_dbg.o
	$(CC)  $(DBG_C_FLAGS) -c $(DEMO_STR_SRC)       -o $(BUILD_DIR)/demo_str_dbg.o
	$(CC)  $(DBG_C_FLAGS) -c $(DEMO_EMBEDDED_SRC)  -o $(BUILD_DIR)/demo_embedded_dbg.o
	$(CXX) $(DBG_CXX_FLAGS) -c $(CPPWRAP_SRC)      -o $(BUILD_DIR)/cpp_wrapper_dbg.o
	$(CXX) $(DBG_CXX_FLAGS) $(DBG_C_OBJS) $(BUILD_DIR)/cpp_wrapper_dbg.o $(ASM_OBJ) -o $@

$(TARGET_OPT): $(ALL_SRCS) $(ASM_OBJ)
	$(CC)  $(OPT_C_FLAGS) -c $(MAIN_SRC)          -o $(BUILD_DIR)/main_opt.o
	$(CC)  $(OPT_C_FLAGS) -c $(MATHLIB_SRC)        -o $(BUILD_DIR)/mathlib_opt.o
	$(CC)  $(OPT_C_FLAGS) -c $(STRUTILS_SRC)       -o $(BUILD_DIR)/strutils_opt.o
	$(CC)  $(OPT_C_FLAGS) -c $(HAL_UART_SRC)       -o $(BUILD_DIR)/hal_uart_opt.o
	$(CC)  $(OPT_C_FLAGS) -c $(HAL_RING_SRC)       -o $(BUILD_DIR)/hal_ring_opt.o
	$(CC)  $(OPT_C_FLAGS) -c $(HAL_SYSTEM_SRC)     -o $(BUILD_DIR)/hal_system_opt.o
	$(CC)  $(OPT_C_FLAGS) -c $(DEMO_MATH_SRC)      -o $(BUILD_DIR)/demo_math_opt.o
	$(CC)  $(OPT_C_FLAGS) -c $(DEMO_STR_SRC)       -o $(BUILD_DIR)/demo_str_opt.o
	$(CC)  $(OPT_C_FLAGS) -c $(DEMO_EMBEDDED_SRC)  -o $(BUILD_DIR)/demo_embedded_opt.o
	$(CXX) $(OPT_CXX_FLAGS) -c $(CPPWRAP_SRC)      -o $(BUILD_DIR)/cpp_wrapper_opt.o
	$(CXX) $(OPT_CXX_FLAGS) -Wl,--gc-sections $(OPT_C_OBJS) $(BUILD_DIR)/cpp_wrapper_opt.o $(ASM_OBJ) -o $@

$(TARGET_STRIP): $(TARGET_OPT)
	cp $< $@
	strip $@

clean:
	rm -rf $(BUILD_DIR)
