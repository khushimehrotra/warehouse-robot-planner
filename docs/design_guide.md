# Design Guide

## 1. Goals

1. A reusable algorithm library with consistent interfaces.
2. A warehouse simulation that uses that library end to end.
3. Evidence: every algorithm verified against an independent oracle, benchmarked, and
   documented.

## 2. Module map

```
                       cli/app.c  (menu + scenario commands)
                      /      |        \
              scenario.c  report.c   strmatch.c, huffman.c (log analysis)
                  |
                sim.c ----------- bnb.c (batch assignment, least-cost B&B)
                  |  \
               grid.c  minheap.c, vec.c
```

| Layer | Modules | Depends on |
|---|---|---|
| Foundations | `vec`, `minheap`, `unionfind`, `grid`, `graph` | nothing (graph uses nothing else) |
| Models and I/O | `model.h`, `task_io`, `robot_io` | `model.h` |
| Algorithms | `sort`, `dnc`, `matrix`, `greedy`, `huffman`, `dp`, `shortest`, `tsp`, `backtrack`, `bnb`, `flow`, `numtheory`, `strmatch` | foundations (`strmatch` uses `dnc` for `power_mod`; `flow` and `shortest` use `graph`; `bnb` uses `vec` and `minheap`) |
| Routing | `gridpath` | `grid`, `graph` |
| System | `sim`, `scenario`, `report` | algorithms and foundations |
| Applications | `cli/main.c` (demo), `cli/app.c` (planner) | system layer |

## 3. Conventions

- **Error codes, not `exit()`.** Library functions return `0` for success and a negative
  number for an error. Typical codes: `-1` bad arguments, `-3` out of memory. Only
  `main` terminates the program. This is what lets tests check invalid inputs.
- **Ownership.** Whoever allocates documents who frees. Loaders (`tasks_load`,
  `robots_load`, `grid_load`) allocate for the caller. Every error path frees what it
  allocated. Free functions set pointers to `NULL`, so freeing twice is safe.
- **One header and one source per module**, with the contract (inputs, outputs, error
  codes) written in the header.
- **Row-major grids.** Cell `(r, c)` is index `r * cols + c`; the same number is the
  graph node id.
- **Tests with oracles.** Each algorithm is compared with a trivial or independent
  method on hundreds of random small inputs. Fixed seeds make failures repeatable.
- **Two builds.** Development and tests use sanitizers. Benchmarks use `-O2` without
  them.

## 4. Data structures

| Structure | Representation | Notes |
|---|---|---|
| `Vec` | `int*`, size, capacity | doubles on growth: amortised O(1) push |
| `MinHeap` | array of `(key, value)` | binary heap, O(log n) push/pop |
| `Graph` | forward-star: `head[u]`, `next[a]`, `to[a]`, `w[a]` | undirected edge = two arcs |
| Flow network | `Graph` with paired arcs `a` and `a^1` | `w[]` holds residual capacity; add edges only with `flow_add_edge` |
| `Grid` | one `char` block | `.`, `#`, `S`, `C` |
| Reservation table | `unsigned short[WIN][cells]` | circular window of `SIM_WIN = 256` ticks |

## 5. Simulation design

### Model

- Time advances in whole ticks. In each tick a robot is in one cell and then moves to a
  4-neighbour or waits.
- Racks (`S`) are not walkable. A robot serves a rack from any adjacent floor cell.
- A task is: go to the shelf, pick (1 tick), go to the destination, drop (1 tick). Then
  the robot returns to its home cell and becomes idle.

### Collision-free planning

- A **reservation table** records which robot occupies which cell at which tick.
- A robot's route is found with **space-time A\***: states are (cell, tick), moves are
  "step" or "wait". The static BFS distance to the goal cells is the heuristic.
- Forbidden: a vertex conflict (a cell reserved at that tick) and a swap conflict (two
  robots exchanging cells).
- Robots plan one after another, so earlier plans have priority: **prioritised
  planning**.
- A robot standing still ("parked") blocks its cell for all future ticks.
- After every tick an **independent checker** counts vertex and swap collisions. With
  reservations on, it must report 0.

### Tick loop (`sim_run`)

1. Release tasks whose release tick has arrived.
2. Assign pending tasks to idle robots (strategy decides).
3. Plan a route for every robot that needs one.
4. Record metrics and the event log for this tick.
5. Advance every robot by one step.
6. Run the collision checker.
7. Stop when every task is finished or failed and all robots are home.

### Assignment strategies

| Strategy | Rule |
|---|---|
| Greedy | Tasks in deadline order, each to the nearest idle robot that can carry it |
| Batch B&B | Up to 8 idle robots and the 8 most urgent tasks, solved exactly with least-cost branch and bound |

### Event log

One character per tick for one robot: `M` move, `W` wait, `P` pick, `D` drop, `I` idle.
The log is searched with the four string matchers and compressed with Huffman coding.

## 6. Branch-and-bound design

The three strategies share all code except the frontier. `fr_push` and `fr_pop` choose
between a queue (FIFO), a stack (LIFO) and a min-heap keyed on the lower bound (least
cost). The bound for a partial assignment is cost so far plus, for each unassigned robot,
its cheapest still-free task. It never overestimates, so pruning when `bound >= best`
is safe.

## 7. Limitations

- Prioritised planning is incomplete. A parked robot in a one-cell corridor can block
  others forever, which appears as `unserved` tasks.
- Battery is tracked, not enforced. There is no charging logic.
- Shocks are static. `sim_run` has no mid-run events. Failing a robot at tick T would
  need an event queue inside `sim.c`.
- `priority` is not used by the planner.
- Grids are limited to 65,536 cells and one leg to 255 ticks (the planning window).
- Held-Karp handles at most 16 cities (memory grows as 2^n).

## 8. Extension ideas

- Mid-run failures and charging: add timed events and a battery check to the tick loop.
- Idle robots retreating to parking cells so they never block aisles.
- Hungarian algorithm for large assignment batches.
- Nearest-neighbour plus 2-opt for tours above 16 stops.
