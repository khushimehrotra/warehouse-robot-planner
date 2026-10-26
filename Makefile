CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude -fsanitize=address,undefined
LIB_SRC = src/vec.c src/minheap.c src/grid.c
TESTS   = tests/test_vec tests/test_minheap tests/test_grid

all: warehouse $(TESTS)

warehouse: cli/main.c $(LIB_SRC)
	$(CC) $(CFLAGS) cli/main.c $(LIB_SRC) -o warehouse

tests/%: tests/%.c $(LIB_SRC)
	$(CC) $(CFLAGS) $< $(LIB_SRC) -o $@

test: $(TESTS)
	@for t in $(TESTS); do ./$$t || exit 1; done

clean:
	rm -f warehouse $(TESTS)