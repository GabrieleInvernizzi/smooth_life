# Bin config
TARGET_NAME := smooth_life
SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := $(BIN_DIR)/obj
INCLUDE_DIR := src
SYSLIBS := m
# Compilation config
CC := gcc
CFLAGS := -Wall -Wextra -std=c17
ifeq ($(debug),true)
	CFLAGS += -O0 -g
else
	CFLAGS += -O3
endif

TARGET := $(BIN_DIR)/$(TARGET_NAME)

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst %.c,%.o,$(SRC:$(SRC_DIR)/%=$(OBJ_DIR)/%))

default: makedir all

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -l$(SYSLIBS) -o $@ $(OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

.PHONY: makedir
makedir:
	@mkdir -p $(dir $(OBJ))

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@echo Project cleaned.
	@rm -fR $(TARGET) $(OBJ_DIR)