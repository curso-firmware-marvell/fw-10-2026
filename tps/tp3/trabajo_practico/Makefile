# Makefile del laboratorio — completar SRCS / LIB_SRCS según README.md (hitos 2+).
# Las reglas de compilación ya están definidas; vos agregás fuentes y flags.

CC        = gcc
AR        = ar
RANLIB    = ranlib
CFLAGS    = -Wall -Wextra -std=c11 -g -O0
CFLAGS   += -ffunction-sections -fdata-sections
OPT_FLAGS = -Wall -Wextra -std=c11 -O2 -DNDEBUG
OPT_FLAGS += -ffunction-sections -fdata-sections
LDFLAGS   = -Wl,--gc-sections
INCLUDES  = -Isrc/lib -Isrc/hal -Isrc/demo -Isrc/app -Isrc/platform
BUILD_DIR = build
TARGET    = $(BUILD_DIR)/building_process_demo
TARGET_OPT= $(BUILD_DIR)/building_process_demo_opt
STATIC_LIB= $(BUILD_DIR)/libutils.a

SRCS  = src/app/main.c

USE_STATIC_LIB = 0
LIB_SRCS =

# Salida de consola: formato común + backend de la plataforma (src/platform/<PLATFORM>/)
PLATFORM      = host
PLATFORM_SRCS = src/platform/print.c $(wildcard src/platform/$(PLATFORM)/*.c)

APP_SRCS      = $(filter src/app/% src/demo/%,$(SRCS))
LIB_OBJS      = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))
OBJS          = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))
APP_OBJS      = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(APP_SRCS))
PLATFORM_OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(PLATFORM_SRCS))

.PHONY: all opt clean verify

all: $(TARGET)

opt: $(TARGET_OPT)

verify:
	@bash verify_lab.sh

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%_opt.o: src/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(OPT_FLAGS) $(INCLUDES) -c $< -o $@

ifeq ($(USE_STATIC_LIB),1)
$(STATIC_LIB): $(LIB_OBJS) | $(BUILD_DIR)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(TARGET): $(APP_OBJS) $(PLATFORM_OBJS) $(STATIC_LIB) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(APP_OBJS) $(PLATFORM_OBJS) -L$(BUILD_DIR) -lutils -o $@
else
$(TARGET): $(OBJS) $(PLATFORM_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
endif

$(TARGET_OPT): $(patsubst src/%.c,$(BUILD_DIR)/%_opt.o,$(SRCS) $(LIB_SRCS) $(PLATFORM_SRCS)) | $(BUILD_DIR)
	$(CC) $(OPT_FLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)
