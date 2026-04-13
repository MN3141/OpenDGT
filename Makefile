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

clean:
	$(MAKE) -C $(TARGET_DIR)/$(TARGET) -f $(TARGET).mk clean