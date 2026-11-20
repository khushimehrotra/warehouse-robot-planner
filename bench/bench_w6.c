#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "strmatch.h"
#include "sim.h"

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

typedef int (*Matcher)(const char *, size_t, const char *, size_t, size_t *, size_t, size_t *);

static void time_matchers(const char *label, const char *t, size_t n, const char *p, size_t m) {
    static const struct { const char *name; Matcher f; } M[] = {
        {"naive", match_naive}, {"rabin-karp", match_rabin_karp},
        {"automaton", match_automaton}, {"kmp", match_kmp}};
    printf("%s (n=%zu, m=%zu)\n", label, n, m);
    for (int i = 0; i < 4; i++) {
        size_t cnt = 0;
        double t0 = now_ms();
        int rc = M[i].f(t, n, p, m, NULL, 0, &cnt);
        double t1 = now_ms();
        printf("  %-11s rc=%d matches=%-8zu %10.3f ms\n", M[i].name, rc, cnt, t1 - t0);
    }
}

/* racks in pairs on every third row, open aisles elsewhere */
static Grid make_grid(int rows, int cols) {
    Grid g;
    g.rows = rows; g.cols = cols;
    g.cells = malloc((size_t)rows * cols);
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            g.cells[r * cols + c] = (r % 3 == 2 && (c % 4 == 1 || c % 4 == 2)) ? 'S' : '.';
    return g;
}

static void run_sim(const char *label, int nr, int nt, SimAssign a, int avoid, int rows, int cols) {
    Grid g = make_grid(rows, cols);
    int nsh = 0;
    for (int i = 0; i < rows * cols; i++) if (g.cells[i] == 'S') nsh++;
    Robot *rb = malloc((size_t)nr * sizeof(Robot));
    Task *tk = malloc((size_t)nt * sizeof(Task));
    for (int i = 0; i < nr; i++) { rb[i].id = i + 1; rb[i].x = i; rb[i].y = 0; rb[i].battery = 100000; rb[i].payload_cap = 10; }
    srand(7);
    for (int i = 0; i < nt; i++) {
        tk[i].id = i + 1; tk[i].shelf_id = rand() % nsh; tk[i].dest_id = rand() % nsh;
        tk[i].weight = 1 + rand() % 5; tk[i].release = i; tk[i].deadline = i + 400; tk[i].priority = 1;
    }
    SimConfig cfg;
    SimMetrics m;
    sim_default_config(&cfg);
    cfg.assign = a;
    cfg.avoid_collisions = avoid;
    double t0 = now_ms();
    int rc = sim_run(&g, rb, nr, tk, nt, &cfg, &m, NULL);
    double t1 = now_ms();
    if (rc == 0)
        printf("%-26s R=%-3d T=%-5d done=%-5d unserved=%-4d makespan=%-6d late=%-5d avg_lat=%-7.1f "
               "dist=%-7lld wait=%-6lld coll=%-4d %9.1f ms\n",
               label, nr, nt, m.completed, m.unserved, m.makespan, m.deadline_misses,
               m.avg_latency, m.total_distance, m.total_wait, m.collisions, t1 - t0);
    else printf("%-26s failed with code %d\n", label, rc);
    free(rb); free(tk); free(g.cells);
}

int main(void) {
    srand(1);
    size_t n = 2000000;
    char *t = malloc(n);
    for (size_t i = 0; i < n; i++) t[i] = (char)('a' + rand() % 4);
    time_matchers("random text, 4 letters", t, n, t + 12345, 8);
    free(t);

    n = 200000;
    t = malloc(n);
    char *p = malloc(1000);
    memset(t, 'a', n);
    memset(p, 'a', 999); p[999] = 'b';
    time_matchers("adversarial: a^n vs a^999 b", t, n, p, 1000);
    memset(p, 'a', 10);
    time_matchers("many matches: a^n vs a^10", t, n, p, 10);
    free(t); free(p);

    printf("\nWarehouse simulation, 40x60 grid with racks\n");
    run_sim("normal  greedy",     10,  200, SIM_ASSIGN_GREEDY, 1, 40, 60);
    run_sim("normal  batch B&B",  10,  200, SIM_ASSIGN_BNB,    1, 40, 60);
    run_sim("normal  NO reserv.", 10,  200, SIM_ASSIGN_GREEDY, 0, 40, 60);
    run_sim("medium  greedy",     20,  500, SIM_ASSIGN_GREEDY, 1, 40, 60);
    run_sim("medium  batch B&B",  20,  500, SIM_ASSIGN_BNB,    1, 40, 60);
    run_sim("large   greedy",     40, 1000, SIM_ASSIGN_GREEDY, 1, 40, 60);
    run_sim("large   batch B&B",  40, 1000, SIM_ASSIGN_BNB,    1, 40, 60);
    return 0;
}