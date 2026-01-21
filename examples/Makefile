ifeq ($(OS),Windows_NT)
	CC := gcc
else
	CC ?= gcc
endif

SRC_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
SRC := $(wildcard $(SRC_DIR)/*.c)

PROJECT_ROOT := $(SRC_DIR)/..

CFLAGS := -ggdb3 -std=c99 -Wall -Wextra -Wconversion -Wpedantic -I$(PROJECT_ROOT)
CFLAGS += -DPROJECT_ROOT=\"$(PROJECT_ROOT)\"
CFLAGS += -DHH_LOG=HH_LOG_DBG

ifeq ($(OS),Windows_NT)
    SUF := .exe
else
    SUF :=
endif

BIN := $(SRC:.c=$(SUF))

$(SRC_DIR)/%$(SUF): $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

all: $(BIN)

.PHONY: all