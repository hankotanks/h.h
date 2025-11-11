CC ?= cc

DIR_TESTS := tests

PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CFLAGS := -ggdb3 -std=c99 -Wall -Wextra -Wconversion -Wpedantic -I$(PROJECT_ROOT)
CFLAGS += -DPROJECT_ROOT=\"$(PROJECT_ROOT)\"

test: $(patsubst $(DIR_TESTS)/%.c, $(DIR_TESTS)/%.o, $(wildcard $(DIR_TESTS)/*.c))
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) -r $(DIR_TESTS)/*.o