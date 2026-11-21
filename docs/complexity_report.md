# Complexity Report

All timings come from `-O2` builds without sanitizers. Raw outputs are saved in `docs/`
(`bench_w2_output.txt`, `bench_w4_output.txt`, `bench_w5_output.txt`,
`bench_w6_output.txt`, `bench_w7.csv`, `curves_output.txt`).
Where this report says *"see file"*, copy the numbers from that file before submitting.

## 1. Recurrences and solutions

| Algorithm | Recurrence | Solution | Method |
|---|---|---|---|
| Merge sort | T(n) = 2T(n/2) + n | Θ(n log n) | Master, case 2 (n^1 = n) |
| Quick sort, average | T(n) = 2T(n/2) + n | Θ(n log n) | Master, case 2 |
| Quick sort, worst | T(n) = T(n-1) + n | Θ(n²) | unrolling |
| Binary search | T(n) = T(n/2) + 1 | Θ(log n) | Master, case 2 |
| Maximum subarray (D&C) | T(n) = 2T(n/2) + n | Θ(n log n) | Master, case 2 |
| Fast powering | T(n) = T(n/2) + 1 | Θ(log n) | Master, case 2 |
| Classical matrix multiply | T(n) = 8T(n/2) + n² | Θ(n³) | Master, case 1 |
| Strassen | T(n) = 7T(n/2) + n² | Θ(n^log₂7) ≈ Θ(n^2.81) | Master, case 1 |
| Celebrity | n-1 eliminations + 2(n-1) checks | Θ(n) | counting |

Master theorem: for T(n) = aT(n/b) + f(n), compare f(n) with n^(log_b a). The larger one
wins; if they are equal, multiply by log n.

## 2. Complexity of each module

| Module | Time | Memory |
|---|---|---|
| Vec push | O(1) amortised | O(n) |
| Min-heap push / pop | O(log n) | O(n) |
| Activity selection | O(n log n) | O(n) |
| Interval partitioning | O(n log n) | O(n) |
| Huffman | O(k log k) for k symbols, O(n) to encode | O(k) |
| Union-find | ≈ O(1) amortised (rank + path compression) | O(n) |
| Kruskal | O(m log m) | O(m) |
| Prim (binary heap) | O((n + m) log n) | O(n + m) |
| Dijkstra | O((n + m) log n) | O(n) |
| Bellman-Ford | O(nm) | O(n) |
| Floyd-Warshall | O(n³) | O(n²) |
| 0/1 knapsack | O(nW), pseudo-polynomial | O(nW) |
| LCS | O(mn) | O(mn) |
| Matrix chain | O(n³) | O(n²) |
| Optimal BST | O(n³) | O(n²) |
| Held-Karp TSP | O(2ⁿ n²) | O(2ⁿ n) |
| N-Queens | exponential, pruned | O(n) |
| Subset sum (backtracking) | O(2ⁿ) worst case, pruned | O(n) |
| Assignment B&B | exponential worst case | O(nodes) |
| Edmonds-Karp | O(V E²) | O(V + E) |
| Kuhn matching | O(V E) | O(V + E) |
| CRT merge | O(n log M) | O(1) |
| Naive match | O(nm) | O(1) |
| Rabin-Karp | O(n + m) expected, O(nm) worst | O(1) |
| Automaton match | O(m·256) build, O(n) search | O(m·256) |
| KMP | O(m) build, O(n) search | O(m) |
| Space-time A* (one leg) | bounded by cells × window | O(cells × window) |

## 3. Correctness evidence

| Algorithm | Oracle |
|---|---|
| Sorts, binary search | C library `qsort`, linear search |
| Max subarray | O(n²) brute force |
| Fast power | repeated multiplication |
| Celebrity | check every person |
| Strassen | classical multiplication (cutoff 1 forces the Strassen formulas) |
| Activity selection | every subset |
| Interval partitioning | maximum overlap depth (also the lower bound) |
| Huffman | round trip encode/decode; never worse than fixed-length code; hand-computed optimum |
| Kruskal, Prim | every edge subset; each other |
| Dijkstra | Floyd-Warshall; BFS on grids |
| Bellman-Ford | Floyd-Warshall; negative-cycle verdicts must agree |
| Knapsack | every subset |
| Matrix chain, optimal BST | plain recursion without a table |
| LCS | every subsequence |
| Held-Karp | every visiting order |
| N-Queens | published solution counts (n = 1..10) |
| Subset sum | every subset |
| Branch and bound | every permutation; all three strategies must give the same optimum |
| Max flow | brute-force minimum cut (max-flow min-cut theorem), flow conservation |
| Bipartite matching | every edge subset; the max-flow reduction |
| CRT | try every value up to the lcm |
| String matchers | the naive matcher; Rabin-Karp also with moduli 7 and 1 to force collisions |
| Simulation | hand-computed corridor run (finish tick, log `MMPMMMMDMM`); collision checker; invariants on random warehouses |

Test counts from the last run include: vec 105, min-heap 1012, sort 30,978, dnc 4,518,
matrix 1,406, greedy 8,473, huffman 1,506, graph 36,901, gridpath 365, dp 5,510,
shortest 85,880, tsp 5,220, backtrack 1,622, bnb 9,998, flow 18,256, numtheory 2,511,
strmatch 32,193, sim 4,074, scenario 5,305, edge 39 checks, all with 0 failures.

## 4. Complexity classes and reductions

**In P:** sorting, shortest paths, minimum spanning tree, maximum flow, bipartite
matching, LCS, matrix chain, optimal BST, string matching, and the assignment problem
(Hungarian algorithm, O(n³)).

**NP-complete (decision versions):** subset sum, 0/1 knapsack, travelling salesman.
Knapsack's O(nW) is pseudo-polynomial: it is polynomial in the *value* of W, not in the
number of bits needed to write W, so it does not contradict NP-completeness.
Held-Karp is exponential and is practical only for about 16 stops.

**NP-hard in this project:** multi-robot path finding with optimal makespan. This is why
the simulator uses a heuristic (prioritised planning) instead of an exact solver.

**Reductions**

1. **Bipartite matching → max flow (implemented).** Add a source joined to every left
   node and a sink joined from every right node, all capacities 1. The maximum flow
   equals the maximum matching. `matching_by_flow` does this, and `test_flow.c` checks
   that it equals Kuhn's augmenting-path result on random graphs.
2. **Subset sum → 0/1 knapsack.** Give each item weight = value = a[i] and capacity =
   target. A subset sums to target exactly when a knapsack of value target exists.
3. **TSP is in NP.** A claimed tour can be verified in O(n) by adding up its edges.

## 5. Empirical results

### Sorting (n = 200,000 unless noted)

| Input | Merge sort | Quick sort |
|---|---|---|
| random | 14.73 ms | 11.00 ms |
| already sorted | 2.67 ms | 4.13 ms |
| all equal (n = 30,000) | 0.44 ms | 104.24 ms |

Quick sort on all-equal data is about 240 times slower: the partition test `a[j] < pivot`
moves nothing, every split is lopsided, and T(n) = T(n-1) + n. A three-way partition
would fix it.

### Matrix multiplication

See `docs/bench_w2_output.txt`. Expect Strassen to be equal to or slower than classical
at these sizes, because of its extra additions and copying, which is why the code
switches to classical below a cutoff. Strassen also needs subtraction, so it cannot be
used for min-plus (shortest-path) products.

### Held-Karp TSP (random distances)

| Cities | Time |
|---|---|
| 14 | 3.4 ms |
| 16 | 16.3 ms |

Each extra city roughly doubles the work or more, as 2ⁿ n² predicts.

### Branch and bound for assignment (random costs 1..99)

| n | Strategy | Generated | Expanded | Time | Optimum |
|---|---|---|---|---|---|
| 9 | FIFO | 623,547 | 260,668 | 34.6 ms | 130 |
| 9 | LIFO | 1,334 | 308 | 0.17 ms | 130 |
| 9 | least-cost | 387 | 74 | 0.09 ms | 130 |
| 10 | FIFO | stopped at the node limit | | | |
| 10 | LIFO | 2,092 | 405 | 0.32 ms | 191 |
| 10 | least-cost | 985 | 154 | 0.27 ms | 191 |

All strategies find the same optimum; only the search effort differs. FIFO finds no
complete solution before the last level, so nothing is pruned. Least-cost always follows
the most promising bound and expands the fewest nodes.

### Matching (200 robots, 200 tasks)

Augmenting paths: 200 pairs in 0.098 ms. Through max flow: 200 pairs in 0.591 ms. Same
answer; the direct method is about 6 times faster.


### String matching (from docs/bench_w6_output.txt)

| Case | Naive | Rabin-Karp | Automaton | KMP |
|---|---|---|---|---|
| random text, 4 letters (n = 2,000,000, m = 8) | 10.26 ms | 34.53 ms | 6.82 ms | 11.26 ms |
| adversarial `a^n` vs `a^999 b` (n = 200,000, m = 1000) | 86.78 ms | 3.27 ms | 1.18 ms | 0.48 ms |
| many matches `a^n` vs `a^10` (199,991 matches) | 0.89 ms | 3.21 ms | 0.64 ms | 0.20 ms |

All four matchers reported identical match counts in every case.

- On the adversarial text naive matching is about 180 times slower than KMP. It
  re-compares up to 1000 characters at every alignment, which is the Θ(nm) worst case.
  KMP, the automaton and Rabin-Karp stay linear.
- On random text mismatches come early, so naive is near its O(n) best case and
  beats KMP and Rabin-Karp. Rabin-Karp is slowest here because it pays for a hash update
  at every position.
- The automaton is the fastest on random text: one table lookup per character, at the
  cost of an m·256 table. KMP wins when many matches overlap.
- Worst-case guarantees are the reason to prefer KMP for event logs. Typical-case speed
  can favour simpler methods.


### Growth rates (from docs/curves_output.txt)

Time ratio when the input size doubles (grid side doubles for Dijkstra, so n quadruples;
Held-Karp adds two cities):

| Algorithm | Measured ratios | Predicted | Verdict |
|---|---|---|---|
| Merge sort | 1.68, 1.92, 2.16, 2.29 | about 2.1 (n log n) | matches; the rise at large n is cache effects |
| Classical matrix multiply | 7.58, 7.49, 8.17 | 8 (n³) | matches |
| Dijkstra on a grid (n x4) | 3.14, 3.96, 4.72 | about 4 to 5 (n log n) | matches |
| Held-Karp (+2 cities) | 5.21, 5.25, 4.88 | about 5 (2ⁿ n²) | matches |
| KMP | 2.09, 1.74, 2.45 | 2 (n) | matches |

Held-Karp reaches 21 ms at 16 cities in this run, which is why 16 is the practical limit.

### Simulation, greedy against batch B&B (reservation table on, 0 collisions everywhere)

| Dataset | Strategy | Makespan | Late | Avg latency | Distance | Wait | Time |
|---|---|---|---|---|---|---|---|
| normal (4 R, 10 T) | greedy | 55 | 0 | 20.4 | 178 | 0 | 0.4 ms |
| normal | batch B&B | 56 | 0 | 20.3 | 168 | 0 | 0.3 ms |
| medium (20 R, 500 T) | greedy | 2784 | 418 | 1259.5 | 53,456 | 22 | 102 ms |
| medium | batch B&B | 2768 | 420 | 1245.8 | 53,194 | 35 | 116 ms |
| large (50 R, 2000 T) | greedy | 4562 | 1773 | 1769.1 | 216,890 | 572 | 565 ms |
| large | batch B&B | 4575 | 1773 | 1768.1 | 216,380 | 613 | 748 ms |
| adversarial (one gap) | greedy | 712 | 38 | 328.1 | 6,582 | 66 | 4.7 ms |
| adversarial | batch B&B | 716 | 37 | 325.9 | 6,592 | 58 | 9.6 ms |

Every run completed all its tasks with `unserved = 0`.

The two strategies are within about 1% on every quality measure. Batch B&B costs about
30% more time on the large run and about twice as much on the adversarial one. With 20
or more robots nearly all robots are busy, so a batch contains only one or two idle
robots and exact assignment has little to optimise. The many late tasks in the large run
come from an under-provisioned fleet (2 tasks released per tick), not from the algorithm.

### Scenario comparison: normal, shock and stress

All runs completed every task (`unserved = 0`). Source: `docs/results_shock.csv`,
`docs/results_stress.csv`.

| Scenario | Robots | Tasks | Strategy | Makespan | Late | Avg latency | Runtime | Peak mem |
|---|---|---|---|---|---|---|---|---|
| normal | 4 | 10 | greedy | 55 | 0 | 20.4 | 0.52 ms | 2 MB |
| normal | 4 | 10 | batch B&B | 56 | 0 | 20.3 | 0.22 ms | 2 MB |
| shock: half the fleet lost | 2 | 10 | greedy | 113 | 0 | 49.6 | 0.18 ms | 2 MB |
| shock: half the fleet lost | 2 | 10 | batch B&B | 113 | 0 | 49.6 | 0.17 ms | 2 MB |
| shock: middle aisle blocked | 4 | 10 | greedy | 82 | 0 | 35.5 | 0.36 ms | 2 MB |
| shock: middle aisle blocked | 4 | 10 | batch B&B | 73 | 0 | 33.7 | 0.22 ms | 2 MB |
| stress | 100 | 10,000 | greedy | 11,448 | 9,391 | 3,215.0 | 13,969 ms | 5.9 MB |
| stress | 100 | 10,000 | batch B&B | 11,385 | 9,394 | 3,194.3 | 13,466 ms | 7.0 MB |

**Capacity shock (fleet halved).** Makespan rose from 55 to 113 (2.05x) and average
latency from 20.4 to 49.6 (2.4x). Throughput scales almost linearly with fleet size, and
both strategies degrade identically because with two robots there is little to choose
between.

**Constraint shock (aisle blocked).** Makespan rose 49% for greedy (55 to 82) but only
30% for batch B&B (56 to 73). This is the one case where exact assignment clearly helped
(about 11% better makespan). A plausible reason is that the blockage makes real
distances differ from the Manhattan estimate that both strategies use, so a globally
optimised batch recovers more than a greedy choice. Treat this as a hypothesis to test,
because it is a single 10-task instance.

**Stress (100 robots, 10,000 tasks).** The planner finished in about 14 seconds
(1.4 ms per task) with every task completed. Batch B&B was 0.5% better on makespan and
3% better on runtime here, so it was not slower. The 9,391 late tasks show the fleet is
under-provisioned for a release rate of 2 tasks per tick (the run needs 11,448 ticks to
finish tasks released within 5,000). That is a capacity result, not an algorithm failure.

**Memory.** Peak memory is a process-wide maximum, so the batch row (run second)
cannot be compared fairly with the greedy row. Measuring each run in its own process
would fix that.

**Collision evidence.** Without the reservation table, the 10-robot, 200-task run
recorded 153 collisions. With it, the checker reports 0.

## 6. Recommended production strategy

| Decision | Recommendation | Reason |
|---|---|---|
| Single-robot routing | Dijkstra (A* as an extension) | non-negative costs, O((n+m) log n) |
| Fixed-map distance tables | Floyd-Warshall, once | O(1) lookups afterwards |
| Negative costs (ramps) | Bellman-Ford | handles negative edges and detects cycles |
| Task assignment | Greedy nearest robot by deadline | within about 1% of exact batch B&B, simpler and faster |
| Large exact assignment | Hungarian algorithm or min-cost flow | assignment is in P; B&B is exponential |
| Pickup tours | Held-Karp up to 16 stops; nearest neighbour + 2-opt above | TSP is NP-hard |
| Collision avoidance | Reservation-table prioritised planning, checker on in tests | collision-free in all runs, scales to 50+ robots |
| Event logs | Huffman for storage, KMP for fault patterns | lossless, linear time |

**Known limits to state honestly:** prioritised planning is incomplete; battery and
charging are not enforced; shocks are static; `priority` is unused.
