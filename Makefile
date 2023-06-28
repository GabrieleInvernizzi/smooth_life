# Bin config
TARGET_NAME := smooth_life
SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := $(BIN_DIR)/obj
INCLUDE_DIR := -Isrc -Iexternal
LIBS := -lm -lncurses
# Compilation config
CC := gcc
CFLAGS := -Wall -Wextra -std=c17 -pthread -fopenmp
ifeq ($(debug),true)
	CFLAGS += -O0 -g
else
	CFLAGS += -O3
endif

TARGET := $(BIN_DIR)/$(TARGET_NAME)

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst %.c,%.o,$(SRC:$(SRC_DIR)/%=$(OBJ_DIR)/%))

# lib srcs and objs
SRC += external/tomlc99/toml.c
OBJ += $(OBJ_DIR)/toml.o

default: makedir all

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c -o $@ $<

$(OBJ_DIR)/toml.o: external/tomlc99/toml.c
	$(CC) -Wall -Wextra -std=c99 -Iexternal/tomlc99 -c -o $@ $<


.PHONY: makedir
makedir:
	@mkdir -p $(dir $(OBJ))

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@echo Project cleaned.
	@rm -fR $(TARGET) $(OBJ_DIR)