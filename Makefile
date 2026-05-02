ifeq ($(OS),Windows_NT)
	CC := gcc
else
	CC ?= gcc
endif

PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CFLAGS := -ggdb3 -std=c99 -Wall -Wextra -Wconversion -Wpedantic -I$(PROJECT_ROOT)
CFLAGS += -DPROJECT_ROOT=\"$(PROJECT_ROOT)\"
CFLAGS += -DHH_LOG=HH_LOG_DBG

ifeq ($(OS),Windows_NT)
    SUF := .exe
else
    SUF :=
endif

TESTS_DIR := $(PROJECT_ROOT)tests
TESTS := $(wildcard $(TESTS_DIR)/*.c)

$(TESTS_DIR)/%$(SUF): $(TESTS_DIR)/%.c h.h
	$(CC) $(CFLAGS) $< -o $@
	@echo "Running $(notdir $@)."
	@$@
	@echo "Exited $(notdir $@) with code $$?."
	@$(RM) $@

h.h: h.h.m4 hcore.h include.h
	m4 $< > $@

test: $(addsuffix $(SUF),$(basename $(TESTS))) h.h

all: h.h

.PHONY: all
