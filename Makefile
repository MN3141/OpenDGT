PROJ_NAME = OpenDGT
TARGET = stm32f411ceu6
TARGET_DIR = target

PROJ_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SRC_DIR = $(PROJ_ROOT)/src/
INC_DIR = $(PROJ_ROOT)/inc/
BUILD_DIR = $(PROJ_ROOT)/build

SRCS = $(wildcard $(SRC_DIR)*.c)

export SRCS
export INC_DIR
export BUILD_DIR

all:
	$(MAKE) -C $(TARGET_DIR)/$(TARGET) -f $(TARGET).mk

flash:
	echo "Flashing..."
	st-flash write $(BUILD_DIR)/$(PROJ_NAME).bin 0x8000000

debug:
	echo "Debugging..."
	gdb-multiarch $(BUILD_DIR)/$(PROJ_NAME).elf -x launch.gdb

clean:
	$(MAKE) -C $(TARGET_DIR)/$(TARGET) -f $(TARGET).mk clean