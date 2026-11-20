#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "grid.h"
#include "robot_io.h"
#include "task_io.h"
#include "sim.h"

static void check_invariants(const SimMetrics *m, const Task *tk, int nt, const int *finish) {
    ASSERT_EQ(m->completed + m->failed + m->unserved, nt);
    ASSERT_EQ(m->collisions, 0);
    int done = 0, late = 0;
    for (int i = 0; i < nt; i++) {
        if (finish[i] < 0) continue;
        done++;
        ASSERT_EQ(finish[i] >= tk[i].release + 2, 1);   /* two dwell ticks at the very least */
        ASSERT_EQ(finish[i] <= m->ticks, 1);
        if (finish[i] > tk[i].deadline) late++;
    }
    ASSERT_EQ(done, m->completed);
    ASSERT_EQ(late, m->deadline_misses);
}

int main(void) {
    SimConfig cfg;
    SimMetrics m;
    int fin[16];
    sim_default_config(&cfg);

    /* ---- exact: one robot, one task, in a corridor ---- */
    {
        char cells[] = "S.....S";
        Grid g = {1, 7, cells};
        Robot r = {1, 3, 0, 100, 5};                    /* id, x, y, battery, payload */
        Task tk = {1, 0, 1, 1, 0, 100, 1};              /* shelf 0 -> shelf 1 */
        char buf[64];
        SimLog lg = {buf, 64, 0, 0};
        cfg.log = &lg;
        ASSERT_EQ(sim_run(&g, &r, 1, &tk, 1, &cfg, &m, fin), 0);
        cfg.log = NULL;
        ASSERT_EQ(m.completed, 1); ASSERT_EQ(m.failed, 0); ASSERT_EQ(m.unserved, 0);
        ASSERT_EQ(fin[0], 8);
        ASSERT_EQ(m.makespan, 8);
        ASSERT_EQ(m.ticks, 10);
        ASSERT_EQ(m.total_distance, 8);
        ASSERT_EQ(m.total_wait, 0);
        ASSERT_EQ(m.collisions, 0);
        ASSERT_EQ(m.deadline_misses, 0);
        ASSERT_EQ(m.max_latency, 8);
        ASSERT_EQ(m.utilization > 0.79 && m.utilization < 0.81, 1);   /* 8 busy ticks of 10 */
        ASSERT_EQ(lg.len, 10);
        ASSERT_EQ(memcmp(buf, "MMPMMMMDMM", 10), 0);
    }

    /* ---- impossible tasks are reported as failed, not as hangs ---- */
    {
        char cells[] = "S.#.S";                         /* a wall cuts the way to shelf 1 */
        Grid g = {1, 5, cells};
        Robot r = {1, 1, 0, 100, 5};
        Task tk = {1, 0, 1, 1, 0, 100, 1};
        ASSERT_EQ(sim_run(&g, &r, 1, &tk, 1, &cfg, &m, fin), 0);
        ASSERT_EQ(m.failed, 1); ASSERT_EQ(m.completed, 0); ASSERT_EQ(fin[0], -1);
        Task heavy = {1, 0, 0, 99, 0, 100, 1};          /* heavier than the robot can carry */
        ASSERT_EQ(sim_run(&g, &r, 1, &heavy, 1, &cfg, &m, fin), 0);
        ASSERT_EQ(m.failed, 1); ASSERT_EQ(m.completed, 0);
    }

    /* ---- two robots, one pickup shelf, open floor: both finish, no collision ---- */
    {
        char cells[] = "S.....S" "......." ".......";
        Grid g = {3, 7, cells};
        Robot r[2] = {{1, 2, 1, 100, 5}, {2, 3, 1, 100, 5}};
        Task tk[2] = {{1, 0, 1, 1, 0, 200, 1}, {2, 0, 1, 1, 0, 200, 1}};
        for (int mode = 0; mode <= 1; mode++) {
            cfg.assign = (SimAssign)mode;
            ASSERT_EQ(sim_run(&g, r, 2, tk, 2, &cfg, &m, fin), 0);
            ASSERT_EQ(m.completed, 2); ASSERT_EQ(m.unserved, 0); ASSERT_EQ(m.collisions, 0);
            check_invariants(&m, tk, 2, fin);
        }
        cfg.avoid_collisions = 0;                       /* no reservations: must still run */
        ASSERT_EQ(sim_run(&g, r, 2, tk, 2, &cfg, &m, fin), 0);
        ASSERT_EQ(m.completed + m.failed + m.unserved, 2);
        cfg.avoid_collisions = 1;
        cfg.assign = SIM_ASSIGN_GREEDY;
    }

    /* ---- the project scenario from the data files ---- */
    {
        Grid g; Robot *rb; Task *tk; int nr, nt;
        ASSERT_EQ(grid_load("data/sim_grid.txt", &g), 0);
        ASSERT_EQ(robots_load("data/sim_robots.txt", &rb, &nr), 0);
        ASSERT_EQ(tasks_load("data/sim_tasks.txt", &tk, &nt), 0);
        ASSERT_EQ(nt, 10);
        int f[32];
        for (int mode = 0; mode <= 1; mode++) {
            cfg.assign = (SimAssign)mode;
            ASSERT_EQ(sim_run(&g, rb, nr, tk, nt, &cfg, &m, f), 0);
            ASSERT_EQ(m.completed, 10); ASSERT_EQ(m.failed, 0); ASSERT_EQ(m.unserved, 0);
            check_invariants(&m, tk, nt, f);
        }
        cfg.assign = SIM_ASSIGN_GREEDY;
        free(rb); free(tk); grid_free(&g);
    }

    /* ---- invalid inputs ---- */
    {
        char cells[] = "S.....S";
        Grid g = {1, 7, cells};
        Robot ok = {1, 3, 0, 100, 5}, onrack = {1, 0, 0, 100, 5}, outside = {1, 9, 0, 100, 5};
        Robot dup[2] = {{1, 3, 0, 100, 5}, {2, 3, 0, 100, 5}};
        Task good = {1, 0, 1, 1, 0, 50, 1}, badshelf = {1, 5, 1, 1, 0, 50, 1};
        Task baddl = {1, 0, 1, 1, 10, 5, 1};
        ASSERT_EQ(sim_run(&g, &onrack, 1, &good, 1, &cfg, &m, NULL), -1);
        ASSERT_EQ(sim_run(&g, &outside, 1, &good, 1, &cfg, &m, NULL), -1);
        ASSERT_EQ(sim_run(&g, dup, 2, &good, 1, &cfg, &m, NULL), -1);
        ASSERT_EQ(sim_run(&g, &ok, 1, &badshelf, 1, &cfg, &m, NULL), -1);
        ASSERT_EQ(sim_run(&g, &ok, 1, &baddl, 1, &cfg, &m, NULL), -1);
        ASSERT_EQ(sim_run(&g, &ok, 0, &good, 1, &cfg, &m, NULL), -1);
        ASSERT_EQ(sim_run(&g, &ok, 1, &good, 1, &cfg, NULL, NULL), -1);
        ASSERT_EQ(sim_run(NULL, &ok, 1, &good, 1, &cfg, &m, NULL), -1);
        ASSERT_EQ(sim_run(&g, &ok, 1, NULL, 0, &cfg, &m, NULL), 0);   /* no tasks is fine */
        ASSERT_EQ(m.completed, 0);
    }

    /* ---- random small warehouses: invariants must always hold ---- */
    srand(131);
    cfg.max_ticks = 600;
    for (int trial = 0; trial < 150; trial++) {
        int rows = 3 + rand() % 4, cols = 3 + rand() % 4, n = rows * cols;
        char cells[64], used[64] = {0};
        for (int i = 0; i < n; i++) cells[i] = (rand() % 100 < 15) ? '#' : '.';
        for (int ns = 0; ns < 3;) {
            int k = rand() % n;
            if (cells[k] != 'S') { cells[k] = 'S'; ns++; }
        }
        Grid g = {rows, cols, cells};
        Robot rb[3];
        int nr = 0;
        for (int tries = 0; tries < 40 && nr < 3; tries++) {
            int k = rand() % n;
            if (cells[k] == '.' && !used[k]) {
                used[k] = 1;
                rb[nr].id = nr + 1; rb[nr].x = k % cols; rb[nr].y = k / cols;
                rb[nr].battery = 100; rb[nr].payload_cap = 3 + rand() % 6;
                nr++;
            }
        }
        if (nr == 0) continue;
        Task tk[6];
        for (int i = 0; i < 6; i++) {
            tk[i].id = i + 1; tk[i].shelf_id = rand() % 3; tk[i].dest_id = rand() % 3;
            tk[i].weight = 1 + rand() % 6; tk[i].release = rand() % 20;
            tk[i].deadline = tk[i].release + 30 + rand() % 60; tk[i].priority = 1;
        }
        for (int mode = 0; mode <= 1; mode++) {
            cfg.assign = (SimAssign)mode;
            ASSERT_EQ(sim_run(&g, rb, nr, tk, 6, &cfg, &m, fin), 0);
            check_invariants(&m, tk, 6, fin);
        }
    }
    TEST_SUMMARY();
}