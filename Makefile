OUT := h.h

ifeq ($(OS),Windows_NT)
	CC := gcc
else
	CC ?= gcc
endif

PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CFLAGS := -ggdb3 -std=c99 -Wall -Wextra -Wconversion -Wpedantic -I$(PROJECT_ROOT) -DPROJECT_ROOT=\"$(PROJECT_ROOT)\"

ifeq ($(OS),Windows_NT)
    SUF := .exe
else
    SUF :=
endif

TESTS_DIR := $(PROJECT_ROOT)tests
TESTS := $(wildcard $(TESTS_DIR)/*.c)

HEADERS_DIR := $(PROJECT_ROOT)include
HEADERS := $(wildcard $(HEADERS_DIR)/*.h)

$(TESTS_DIR)/%$(SUF): $(TESTS_DIR)/%.c h.h
	@$(CC) $(CFLAGS) $< -o $@ 
	@echo "[$(notdir $@)] started"
	@$@
	@echo "[$(notdir $@)] exited with code $$?"
	@$(RM) $@

$(OUT): $(OUT).m4 $(HEADERS)
	m4 $< > $@

test: $(addsuffix $(SUF),$(basename $(TESTS))) $(OUT)

all: $(OUT)

.PHONY: all test
