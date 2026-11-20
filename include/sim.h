#ifndef SIM_H
#define SIM_H
#include "grid.h"
#include "model.h"

#define SIM_WIN 256   /* planning window: one leg may take at most SIM_WIN-1 ticks */

typedef enum { SIM_ASSIGN_GREEDY = 0, SIM_ASSIGN_BNB = 1 } SimAssign;

/* Optional event log of ONE robot: one character per tick.
   M move, W wait (blocked or waiting for a plan), P pick, D drop, I idle. */
typedef struct { char *buf; int cap; int len; int robot; } SimLog;

typedef struct {
    SimAssign assign;
    int avoid_collisions;   /* 1 = reservation table, 0 = every robot plans as if alone */
    int max_ticks;          /* hard stop, >= 1 */
    int slack;              /* extra ticks a conflict-free plan may take, >= 0 */
    SimLog *log;            /* may be NULL */
} SimConfig;

typedef struct {
    int ticks;                       /* ticks simulated */
    int makespan;                    /* tick at which the last task finished */
    int completed, failed, unserved; /* failed = impossible (too heavy / unreachable) */
    int deadline_misses;
    double avg_latency;              /* finish - release, over completed tasks */
    int max_latency;
    long long total_distance;        /* cell moves by all robots, including trips home */
    long long total_wait;            /* ticks a robot holding a task stood still */
    double utilization;              /* robot-ticks holding a task / (robots * ticks) */
    int collisions;                  /* vertex + swap collisions found by the checker */
    long long plan_calls, plan_failures;
} SimMetrics;

void sim_default_config(SimConfig *c);

/* Robots start on distinct walkable non-'S' cells (x = column, y = row).
   Tasks need valid shelf ids, weight > 0, release >= 0, deadline >= release.
   finish[i] (may be NULL, else nt ints) = tick task i finished, or -1.
   Grids of up to 65536 cells.
   Return: 0 ok, -1 bad arguments, -3 out of memory. */
int sim_run(const Grid *g, const Robot *robots, int nr, const Task *tasks, int nt,
            const SimConfig *cfg, SimMetrics *out, int *finish);

#endif