# Makefile for STM8S project with SDCC compiler

# Compiler and flags
CC = sdcc
CFLAGS = -lstm8 -mstm8 --out-fmt-ihx --std-sdcc11

# Source and build directories
SRC_DIR = src
BUILD_DIR = build

# Source files (add more if needed)
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.rel)

# Output binary
TARGET = $(BUILD_DIR)/main.ihx

DEBUG = 0

# Default target
all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -DUSE_UART=${DEBUG}

clean:
	rm -rf $(BUILD_DIR)

debug: clean
	@$(MAKE) DEBUG=1 $(TARGET)

flash:
	sudo stm8flash -c stlinkv21 -p stm8s003f3 -w $(BUILD_DIR)/main.ihx
#	 sudo stm8flash -c stlinkv21 -p stm8s207k8 -w $(BUILD_DIR)/main.ihx

.PHONY: all clean debug
