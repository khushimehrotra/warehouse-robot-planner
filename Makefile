CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude -fsanitize=address,undefined
LIB_SRC = src/vec.c src/minheap.c src/grid.c src/task_io.c src/robot_io.c src/sort.c src/dnc.c src/matrix.c src/greedy.c src/huffman.c src/unionfind.c src/graph.c src/gridpath.c
TESTS   = tests/test_vec tests/test_minheap tests/test_grid tests/test_tasks tests/test_robots tests/test_sort tests/test_dnc tests/test_matrix tests/test_greedy tests/test_huffman tests/test_graph tests/test_gridpath

all: warehouse $(TESTS)

warehouse: cli/main.c $(LIB_SRC)
	$(CC) $(CFLAGS) cli/main.c $(LIB_SRC) -o warehouse

tests/%: tests/%.c $(LIB_SRC)
	$(CC) $(CFLAGS) $< $(LIB_SRC) -o $@

test: $(TESTS)
	@for t in $(TESTS); do ./$$t || exit 1; done

clean:
	rm -f warehouse $(TESTS)

bench_w2: bench/bench_w2.c $(LIB_SRC)
	$(CC) -O2 -Wall -Wextra -Iinclude bench/bench_w2.c $(LIB_SRC) -o bench_w2