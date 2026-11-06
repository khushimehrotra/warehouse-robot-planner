# Warehouse Robot Path and Task Planner

A C implementation of a warehouse robotics planner: task allocation,
collision-aware path planning, and a library of classic algorithms
(sorting, divide and conquer, greedy, dynamic programming, graph and flow
algorithms, string matching), each tested against a brute-force oracle.

## Features (so far)
- Reusable data structures: growable array, min-heap, grid loader
- Merge sort, quick sort, binary search (iterative and recursive)
- Max subarray, fast modular power, celebrity problem
- Classical and Strassen matrix multiplication
- Unit and randomized tests, AddressSanitizer enabled
- Benchmark runner (quick sort vs merge sort, Strassen vs classical)
- Dijkstra route planning on the warehouse grid, verified against BFS
- Greedy scheduling (activity selection, interval partitioning), Huffman coding, Kruskal and Prim MST

## Build and test (Linux or WSL)
    make
    make test
    make bench_w2 && ./bench_w2

## Project layout
    include/  headers        src/    implementations
    tests/    test programs  bench/  benchmarks
    data/     sample inputs  docs/   notes and results

## Roadmap
Greedy algorithms and Dijkstra routing, dynamic programming, backtracking
and branch-and-bound, max-flow and matching, string matching, simulation
engine, CLI, final report.
