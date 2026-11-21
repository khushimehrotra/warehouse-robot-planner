# Warehouse Robot Path and Task Planner

A portable C implementation of a warehouse robotics planner. Robots receive pickup
tasks, plan collision-free routes on a grid map, and are measured on service
indicators such as makespan, latency, deadline misses and utilisation.

Underneath is a reusable library of the classic algorithms from an algorithms course
(sorting, divide and conquer, greedy, dynamic programming, backtracking, flow, number
theory, string matching). Every algorithm is tested against an independent oracle: a
brute-force search, a second algorithm, or a published answer.

## Highlights

- **23 test programs**, all passing, built with AddressSanitizer and UBSan.
- **Collision-aware simulation**: space-time A* over a reservation table (prioritised
  planning) plus an independent collision checker that reports 0 collisions in every
  benchmark run.
- **Two assignment strategies compared on identical input**: greedy nearest robot, and
  batch exact assignment using least-cost branch and bound.
- **Three scenarios**: normal operation, capacity/constraint shock, large-data stress.
- **Menu-driven CLI** with CSV and JSON export and a benchmark runner covering small,
  medium, large and adversarial data.

## Quick start (Linux, or Windows through WSL)

```bash
sudo apt install build-essential git      # once
make                                      # build everything (sanitizers on)
make test                                 # run all 23 test programs
make regress                              # compare demo output with stored output
make planner_fast                         # optimised build for timing
./planner_fast normal                     # scenario 1
./planner_fast shock                      # scenario 2
./planner_fast stress 100 10000           # scenario 3 (robots tasks)
./planner_fast bench                      # small / medium / large / adversarial
./planner_fast                            # interactive menu
./scripts/demo.sh                         # the whole demonstration
```

Run every command from the project root: data paths are relative.

## Sample result (same input, two strategies)

| Dataset | Strategy | Done | Collisions | Makespan | Avg latency | Time |
|---|---|---|---|---|---|---|
| normal (4 robots, 10 tasks) | greedy | 10/10 | 0 | 55 | 20.4 | 0.4 ms |
| normal | batch B&B | 10/10 | 0 | 56 | 20.3 | 0.3 ms |
| large (50 robots, 2000 tasks) | greedy | 2000/2000 | 0 | 4562 | 1769.1 | 565 ms |
| large | batch B&B | 2000/2000 | 0 | 4575 | 1768.1 | 748 ms |

Finding: the exact assignment gives almost no quality gain (within about 1%) and costs
more time, so greedy is the recommended default. See the complexity report.

## What is implemented

| Area | Modules |
|---|---|
| Data structures | growable array, min-heap, union-find, grid, graph (forward-star) |
| Sorting and search | merge sort, quick sort (random pivot), binary search (iterative and recursive) |
| Divide and conquer | maximum subarray, fast modular power, celebrity problem, classical and Strassen matrix multiplication |
| Greedy | activity selection, interval partitioning, Huffman coding, Dijkstra, Prim, Kruskal |
| Dynamic programming | 0/1 knapsack, matrix chain, LCS, optimal BST, Bellman-Ford, Floyd-Warshall, Held-Karp TSP |
| Backtracking and B&B | N-Queens, subset sum, FIFO / LIFO / least-cost branch and bound for assignment |
| Flow and matching | Ford-Fulkerson (Edmonds-Karp), min cut, bipartite matching, matching-to-flow reduction |
| Number theory | extended gcd, modular inverse, Chinese Remainder Theorem (non-coprime moduli) |
| Strings | naive, Rabin-Karp, finite automaton, KMP |
| Simulation | tick loop, reservation table, two strategies, collision checker, metrics, event log |
| Tooling | scenario builder, CSV/JSON export, menu app, benchmark runners |

## Project layout

```
include/   headers (one per module)        src/       implementations
cli/       main.c (demo), app.c (planner)  tests/     test programs + regression/
bench/     benchmark programs              data/      sample maps, robots, tasks
docs/      guides, report, saved results   scripts/    demo.sh
```

## Documentation

- [User guide](docs/user_guide.md): build, run, file formats, exports
- [Design guide](docs/design_guide.md): modules, conventions, simulation design
- [Complexity report](docs/complexity_report.md): recurrences, empirical results,
  correctness evidence, P/NP discussion, reductions, production strategy

## Known limitations

- Prioritised planning is not complete: in a narrow corridor a parked robot can block
  others, and those tasks end as `unserved`.
- Battery is tracked as distance but not enforced; there is no charging logic.
- Shock scenarios are static (a degraded warehouse is compared with the baseline);
  failures in the middle of a run are not simulated.
- Task `priority` is stored but not used by the planner.
- Inputs are plain text files. Results can be exported as CSV or JSON.

## Testing approach

Each algorithm is checked against an oracle on hundreds of random small inputs, plus
edge cases (empty, single element, duplicates, zero capacity, disconnected graphs,
overflow) and invalid inputs (library functions return error codes and never exit).
All tests run with `-fsanitize=address,undefined`. Timing runs use `-O2` without
sanitizers.

## Version

v1.0
