#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include "scenario.h"
#include "sim.h"
#include "report.h"
#include "strmatch.h"
#include "huffman.h"

#define MAXROWS 64

typedef struct {
    Scenario sc;
    int loaded;
    int mode;                  /* 0 compare both strategies, 1 greedy only, 2 batch B&B only */
    int avoid;                 /* 1 = reservation table on */
    ResultRow rows[MAXROWS];
    int nrows;
} App;

static App A;

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static long rss_kb(void) {                      /* peak resident memory, kilobytes on Linux */
    struct rusage ru;
    return getrusage(RUSAGE_SELF, &ru) == 0 ? ru.ru_maxrss : 0;
}

static const char *strat_name(SimAssign as, int avoid) {
    if (!avoid) return as ? "bnb-noreserve" : "greedy-noreserve";
    return as ? "batch-bnb" : "greedy";
}

static void ask_str(const char *prompt, char *buf, size_t n, const char *def) {
    printf("%s [%s]: ", prompt, def);
    if (!fgets(buf, (int)n, stdin)) { snprintf(buf, n, "%s", def); return; }
    buf[strcspn(buf, "\r\n")] = '\0';
    if (buf[0] == '\0') snprintf(buf, n, "%s", def);
}

static long ask_num(const char *prompt, long def) {
    char b[64], *e;
    printf("%s [%ld]: ", prompt, def);
    if (!fgets(b, sizeof b, stdin)) return def;
    long v = strtol(b, &e, 10);
    return e == b ? def : v;
}

static void print_row(const ResultRow *r) {
    const SimMetrics *m = &r->m;
    printf("%-11s %-16s R=%-3d T=%-5d done=%-5d fail=%-3d unserved=%-4d makespan=%-6d late=%-5d "
           "lat=%-7.1f dist=%-7lld wait=%-6lld coll=%-3d %9.1f ms %5ld MB\n",
           r->scenario, r->strategy, r->nr, r->nt, m->completed, m->failed, m->unserved,
           m->makespan, m->deadline_misses, m->avg_latency, m->total_distance,
           m->total_wait, m->collisions, r->ms, r->mem_kb / 1024);
}

static int run_one(App *a, const Scenario *sc, const char *name, SimAssign as,
                   int max_ticks, SimLog *log) {
    SimConfig cfg;
    SimMetrics m;
    sim_default_config(&cfg);
    cfg.assign = as;
    cfg.avoid_collisions = a->avoid;
    cfg.max_ticks = max_ticks;
    cfg.log = log;
    double t0 = now_ms();
    int rc = sim_run(&sc->g, sc->rb, sc->nr, sc->tk, sc->nt, &cfg, &m, NULL);
    double t1 = now_ms();
    if (rc != 0) {
        printf("%-11s %-16s failed (code %d)\n", name, strat_name(as, a->avoid), rc);
        return rc;
    }
    if (a->nrows < MAXROWS) {
        ResultRow *r = &a->rows[a->nrows++];
        snprintf(r->scenario, sizeof r->scenario, "%s", name);
        snprintf(r->strategy, sizeof r->strategy, "%s", strat_name(as, a->avoid));
        r->nr = sc->nr; r->nt = sc->nt; r->m = m; r->ms = t1 - t0; r->mem_kb = rss_kb();
        print_row(r);
    }
    return 0;
}

static void run_set(App *a, const Scenario *sc, const char *name, int max_ticks) {
    for (int st = 0; st < 2; st++) {
        if ((a->mode == 1 && st == 1) || (a->mode == 2 && st == 0)) continue;
        run_one(a, sc, name, (SimAssign)st, max_ticks, NULL);
    }
}

static int do_load(App *a, const char *g, const char *r, const char *t) {
    Scenario n;
    int rc = scenario_load(&n, g, r, t);
    if (rc != 0) { printf("Could not load the data (code %d)\n", rc); return rc; }
    scenario_free(&a->sc);
    a->sc = n;
    a->loaded = 1;
    printf("Loaded a %dx%d grid, %d robots, %d tasks\n", n.g.rows, n.g.cols, n.nr, n.nt);
    return 0;
}

static void scn_normal(App *a) {
    if (!a->loaded) { printf("Load data first (option 1).\n"); return; }
    printf("\n-- NORMAL OPERATION --\n");
    run_set(a, &a->sc, "normal", 100000);
}

static void scn_shock(App *a) {
    if (!a->loaded) { printf("Load data first (option 1).\n"); return; }
    int lose = a->sc.nr / 2;
    if (lose < 1) { printf("Need at least 2 robots for a fleet shock.\n"); return; }
    printf("\n-- CAPACITY / CONSTRAINT SHOCK (baseline, then %d robots lost, then aisle blocked) --\n", lose);
    run_set(a, &a->sc, "normal", 100000);
    Scenario f, b;
    if (scenario_shock_fleet(&a->sc, &f, lose) == 0) {
        run_set(a, &f, "shock-fleet", 100000);
        scenario_free(&f);
    }
    if (scenario_shock_block(&a->sc, &b) == 0) {
        run_set(a, &b, "shock-aisle", 100000);
        scenario_free(&b);
    }
}

static void scn_stress(App *a, int nr, int nt) {
    Scenario s;
    printf("\n-- STRESS: %d robots, %d tasks, 40x60 grid --\n", nr, nt);
    int rc = scenario_generate(&s, 40, 60, nr, nt, 1);
    if (rc != 0) { printf("Could not build the scenario (code %d; robots must be <= 120)\n", rc); return; }
    run_set(a, &s, "stress", 60000);
    scenario_free(&s);
}

static void scn_log(App *a) {
    if (!a->loaded) { printf("Load data first (option 1).\n"); return; }
    static char buf[20000];
    SimLog lg = {buf, (int)sizeof buf, 0, 0};
    if (run_one(a, &a->sc, "log", SIM_ASSIGN_GREEDY, 100000, &lg) != 0) return;
    printf("event log of robot 1 (%d ticks), first 200: %.*s\n", lg.len, lg.len > 200 ? 200 : lg.len, buf);
    static const struct { const char *name;
        int (*f)(const char *, size_t, const char *, size_t, size_t *, size_t, size_t *); } M[] = {
        {"naive", match_naive}, {"rabin-karp", match_rabin_karp},
        {"automaton", match_automaton}, {"kmp", match_kmp}};
    const char *pat[2] = {"WW", "PM"};
    for (int p = 0; p < 2; p++) {
        printf("pattern \"%s\":", pat[p]);
        for (int i = 0; i < 4; i++) {
            size_t cnt = 0;
            M[i].f(buf, (size_t)lg.len, pat[p], 2, NULL, 0, &cnt);
            printf("  %s=%zu", M[i].name, cnt);
        }
        printf("\n");
    }
    Huff h;
    if (lg.len > 0 && huff_build(&h, (const unsigned char *)buf, (size_t)lg.len) == 0)
        printf("log size: %d bits plain, %zu bits Huffman-coded\n", 8 * lg.len,
               huff_bits(&h, (const unsigned char *)buf, (size_t)lg.len));
}

static void scn_bench(App *a) {
    static const struct { const char *name; int kind, rows, cols, nr, nt; } D[] = {
        {"small", 0, 20, 30, 5, 50}, {"medium", 0, 40, 60, 20, 500},
        {"large", 0, 40, 60, 50, 2000}, {"adversarial", 1, 15, 30, 10, 100}};
    int save = a->mode;
    a->mode = 0;
    a->nrows = 0;
    printf("\n-- BENCHMARK (greedy vs batch B&B; peak memory is a process-wide maximum) --\n");
    for (int i = 0; i < 4; i++) {
        Scenario s;
        int rc = D[i].kind == 0 ? scenario_generate(&s, D[i].rows, D[i].cols, D[i].nr, D[i].nt, 1)
                                : scenario_bottleneck(&s, D[i].cols, D[i].nr, D[i].nt, 1);
        if (rc != 0) { printf("%s: could not build (code %d)\n", D[i].name, rc); continue; }
        run_set(a, &s, D[i].name, 30000);
        scenario_free(&s);
    }
    a->mode = save;
    printf("saved docs/bench_w7.csv (code %d)\n", report_write_csv("docs/bench_w7.csv", a->rows, a->nrows));
}

static void export_all(App *a, const char *stem) {
    char p1[160], p2[160];
    snprintf(p1, sizeof p1, "%s.csv", stem);
    snprintf(p2, sizeof p2, "%s.json", stem);
    int r1 = report_write_csv(p1, a->rows, a->nrows);
    int r2 = report_write_json(p2, a->rows, a->nrows);
    printf("wrote %s (code %d) and %s (code %d)\n", p1, r1, p2, r2);
}

static void menu(App *a) {
    for (;;) {
        printf("\n=== Warehouse Robot Planner ===\n"
               " 1) Load data files          2) Normal operation\n"
               " 3) Shock scenario           4) Stress scenario\n"
               " 5) Strategy: %-14s 6) Collision table: %s\n"
               " 7) Event-log analysis       8) Show results so far (%d)\n"
               " 9) Export CSV + JSON       10) Benchmark runner\n"
               " 0) Quit\n> ",
               a->mode == 0 ? "compare both" : (a->mode == 1 ? "greedy only" : "batch B&B only"),
               a->avoid ? "ON " : "OFF", a->nrows);
        char line[64];
        if (!fgets(line, sizeof line, stdin)) return;
        switch (atoi(line)) {
        case 1: {
            char g[128], r[128], t[128];
            ask_str("grid file", g, sizeof g, "data/sim_grid.txt");
            ask_str("robots file", r, sizeof r, "data/sim_robots.txt");
            ask_str("tasks file", t, sizeof t, "data/sim_tasks.txt");
            do_load(a, g, r, t);
            break;
        }
        case 2: scn_normal(a); break;
        case 3: scn_shock(a); break;
        case 4: {
            long nr = ask_num("robots (1..120)", 100), nt = ask_num("tasks", 10000);
            if (nr < 1 || nr > 120 || nt < 0 || nt > 1000000) printf("Out of range.\n");
            else scn_stress(a, (int)nr, (int)nt);
            break;
        }
        case 5: a->mode = (a->mode + 1) % 3; break;
        case 6: a->avoid = !a->avoid; break;
        case 7: scn_log(a); break;
        case 8: for (int i = 0; i < a->nrows; i++) print_row(&a->rows[i]); break;
        case 9: { char s[128]; ask_str("file name without extension", s, sizeof s, "docs/results");
                  export_all(a, s); break; }
        case 10: scn_bench(a); break;
        case 0: return;
        default: printf("Unknown choice.\n");
        }
    }
}

int main(int argc, char *argv[]) {
    A.avoid = 1;
    A.mode = 0;
    int rc = 0;
    if (argc > 1) {
        const char *cmd = argv[1];
        if (strcmp(cmd, "bench") == 0) { scn_bench(&A); return 0; }
        if (strcmp(cmd, "normal") && strcmp(cmd, "shock") && strcmp(cmd, "stress")) {
            printf("usage: %s [normal | shock | stress [robots tasks] | bench]\n", argv[0]);
            return 2;
        }
        if (strcmp(cmd, "stress") != 0 &&
            do_load(&A, "data/sim_grid.txt", "data/sim_robots.txt", "data/sim_tasks.txt") != 0) return 1;
        if (strcmp(cmd, "normal") == 0) scn_normal(&A);
        else if (strcmp(cmd, "shock") == 0) scn_shock(&A);
        else scn_stress(&A, argc > 3 ? atoi(argv[2]) : 100, argc > 3 ? atoi(argv[3]) : 10000);
        char stem[64];
        snprintf(stem, sizeof stem, "docs/results_%s", cmd);
        export_all(&A, stem);
    } else {
        menu(&A);
    }
    scenario_free(&A.sc);
    return rc;
}