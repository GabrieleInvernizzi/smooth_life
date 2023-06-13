# Bin config
TARGET_NAME := smooth_life
SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := $(BIN_DIR)/obj
INCLUDE_DIR := src
LIBS := -lm
# Compilation config
CC := gcc
CFLAGS := -Wall -Wextra -std=c17
ifeq ($(debug),true)
	CFLAGS += -O0 -g
else
	CFLAGS += -O3
endif

TARGET := $(BIN_DIR)/$(TARGET_NAME)

#SRC := $(wildcard $(SRC_DIR)/*.c)

SRC := main.c smooth_life.c
ifeq ($(ncurses),true)
	LIBS += -lncurses
	SRC += ncurses_renderer.c
	DEFINES := NCURSES_RENDERER
else
	SRC += term_renderer.c
	DEFINES := TERM_RENDERER
endif
SRC := $(SRC:%=$(SRC_DIR)/%)
OBJ := $(patsubst %.c,%.o,$(SRC:$(SRC_DIR)/%=$(OBJ_DIR)/%))

default: makedir all

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -D$(DEFINES) -I$(INCLUDE_DIR) -c -o $@ $<

.PHONY: makedir
makedir:
	@mkdir -p $(dir $(OBJ))

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@echo Project cleaned.
	@rm -fR $(TARGET) $(OBJ_DIR)