# User Guide

## 1. Requirements

- A C compiler (`gcc`), `make` and `git`
- Linux, macOS, or Windows with WSL (AddressSanitizer works best on Linux)

```bash
sudo apt update
sudo apt install build-essential git
```

On Windows, open the project in VS Code, type `wsl` in the terminal, and run all
commands there. Run every command from the project root (`warehouse/`).

## 2. Build, test, run

| Command | What it does |
|---|---|
| `make` | Builds `warehouse` (demo) and all test programs, with sanitizers |
| `make test` | Runs all test programs; each prints `N checks, 0 failed` |
| `make regress` | Compares the demo output with the stored expected output |
| `make planner` | Builds the planner with sanitizers (for debugging) |
| `make planner_fast` | Builds the planner with `-O2` (use this for timing) |
| `make bench_w2 bench_w4 bench_w5 bench_w6 bench_curves` | Builds the benchmark programs |
| `make clean` | Removes built files |

After editing a header, run `make clean` first: the Makefile watches `.c` files only.

## 3. The planner

```
./planner_fast normal             normal operation on data/sim_*.txt
./planner_fast shock              baseline, then half the robots lost, then aisle blocked
./planner_fast stress [R T]       generated 40x60 warehouse with R robots, T tasks
                                  (default 100 robots, 10000 tasks, R must be <= 120)
./planner_fast bench              small, medium, large and adversarial datasets
./planner_fast                    interactive menu
```

Every command prints one line per strategy and writes `docs/results_<name>.csv` and
`.json` (the benchmark writes `docs/bench_w7.csv`).

### Menu options

| Key | Action |
|---|---|
| 1 | Load a grid, robot and task file (defaults: `data/sim_*.txt`) |
| 2 | Normal operation |
| 3 | Shock scenario |
| 4 | Stress scenario (asks for robots and tasks) |
| 5 | Cycle strategy: compare both, greedy only, batch B&B only |
| 6 | Turn the collision reservation table on or off |
| 7 | Event-log analysis (pattern search with four matchers, Huffman size) |
| 8 | Show results collected so far |
| 9 | Export CSV and JSON |
| 10 | Benchmark runner |
| 0 | Quit |

Turning the reservation table off (option 6) lets you see why it is needed: robots then
plan as if alone and the collision count becomes greater than zero.

### Reading the result line

```
normal  greedy  R=4 T=10 done=10 fail=0 unserved=0 makespan=55 late=0 lat=20.4 dist=178 wait=0 coll=0
```

| Field | Meaning |
|---|---|
| R, T | robots, tasks |
| done | tasks completed |
| fail | tasks impossible (too heavy for every robot, or unreachable) |
| unserved | tasks not finished when the tick limit was reached |
| makespan | tick at which the last task finished |
| late | tasks finished after their deadline |
| lat | average of (finish tick - release tick) |
| dist | total cell moves by all robots, including trips home |
| wait | ticks a robot holding a task stood still |
| coll | collisions found by the checker (must be 0 with reservations) |

## 4. Input files

All files are plain text. Use LF line endings (`sed -i 's/\r$//' file` fixes Windows files).

### Grid (`data/sim_grid.txt`)

First line `rows cols`, then one line per row.

| Symbol | Meaning |
|---|---|
| `.` | free floor |
| `#` | obstacle |
| `S` | rack/shelf (robots cannot enter; they serve it from an adjacent floor cell) |
| `C` | charging station (treated as floor) |

Shelf ids count `S` cells row by row starting at 0.

### Robots (`data/sim_robots.txt`)

First line is the count, then `id x y battery payload_cap` per robot. `x` is the column,
`y` the row. Robots must start on distinct floor cells that are not `S`.

### Tasks (`data/sim_tasks.txt`)

First line is the count, then `id shelf_id dest_id weight release deadline priority`.
Weight must be positive and deadline must not be before release.

## 5. CSV and JSON columns

`scenario, strategy, robots, tasks, completed, failed, unserved, makespan,
deadline_misses, avg_latency, max_latency, total_distance, total_wait, utilization,
collisions, plan_calls, plan_failures, runtime_ms, peak_mem_kb`

`peak_mem_kb` is the peak memory of the whole process so far, so later rows can only
show equal or larger values. To measure one run alone, run it in its own process.

## 6. The demo program

`./warehouse` prints the sample map, the shortest route from the charger to a shelf, and
the best pickup tour found by Held-Karp. `./warehouse sim` runs the simulation demo and
the event-log analysis.

## 7. Troubleshooting

| Problem | Fix |
|---|---|
| `make: command not found` in PowerShell | Type `wsl` first |
| `missing separator` in Makefile | Command lines must start with a Tab |
| `undefined reference to main` | The `.c` file is empty or unsaved; save it and rebuild |
| `undefined reference to <function>` | The source file is missing from `LIB_SRC` in the Makefile |
| Data file fails to load | Check line endings (CRLF) and that you run from the project root |
| Timings look too slow | Use `planner_fast`, not `planner` (sanitizers slow code a lot) |
| `unserved > 0` in a run | A known limit of prioritised planning (see the design guide) |
