#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "scenario.h"
#include "report.h"
#include "sim.h"

static int count_char(const Grid *g, char ch) {
    int n = 0;
    for (int i = 0; i < g->rows * g->cols; i++) if (g->cells[i] == ch) n++;
    return n;
}

static void sane(const SimMetrics *m, int nt) {
    ASSERT_EQ(m->completed + m->failed + m->unserved, nt);
    ASSERT_EQ(m->collisions, 0);
}

int main(void) {
    Scenario s, f, b, g;
    SimConfig cfg;
    SimMetrics m;
    sim_default_config(&cfg);
    cfg.max_ticks = 3000;

    /* ---- load + shocks on the project data ---- */
    ASSERT_EQ(scenario_load(&s, "data/sim_grid.txt", "data/sim_robots.txt", "data/sim_tasks.txt"), 0);
    ASSERT_EQ(s.nr, 4); ASSERT_EQ(s.nt, 10); ASSERT_EQ(s.g.rows, 9);

    ASSERT_EQ(scenario_shock_fleet(&s, &f, 2), 0);
    ASSERT_EQ(f.nr, 2); ASSERT_EQ(f.nt, 10); ASSERT_EQ(f.rb[1].id, 2);
    ASSERT_EQ(f.rb != s.rb, 1);                          /* a real copy */
    ASSERT_EQ(sim_run(&f.g, f.rb, f.nr, f.tk, f.nt, &cfg, &m, NULL), 0);
    sane(&m, f.nt);
    scenario_free(&f);
    ASSERT_EQ(scenario_shock_fleet(&s, &f, 4), -1);      /* cannot lose everyone */
    ASSERT_EQ(scenario_shock_fleet(&s, &f, -1), -1);

    ASSERT_EQ(scenario_shock_block(&s, &b), 0);
    ASSERT_EQ(count_char(&s.g, '#'), 0);
    ASSERT_EQ(count_char(&b.g, '#'), 8);                 /* row 4, columns 2..9 */
    ASSERT_EQ(sim_run(&b.g, b.rb, b.nr, b.tk, b.nt, &cfg, &m, NULL), 0);
    sane(&m, b.nt);
    scenario_free(&b);
    scenario_free(&s);
    scenario_free(&s);                                   /* freeing twice must be safe */

    ASSERT_EQ(scenario_load(&s, "data/nope.txt", "data/sim_robots.txt", "data/sim_tasks.txt"), -1);
    ASSERT_EQ(s.g.cells == NULL, 1);

    /* ---- generated stress data ---- */
    ASSERT_EQ(scenario_generate(&g, 40, 60, 100, 10000, 1), 0);
    ASSERT_EQ(g.nr, 100); ASSERT_EQ(g.nt, 10000);
    int nsh = count_char(&g.g, 'S');
    ASSERT_EQ(nsh > 0, 1);
    for (int i = 0; i < g.nr; i++) {
        ASSERT_EQ(grid_get(&g.g, g.rb[i].y, g.rb[i].x), '.');
        for (int j = i + 1; j < g.nr; j++)
            ASSERT_EQ(g.rb[i].x != g.rb[j].x || g.rb[i].y != g.rb[j].y, 1);
    }
    for (int i = 0; i < g.nt; i += 97) {
        ASSERT_EQ(g.tk[i].shelf_id >= 0 && g.tk[i].shelf_id < nsh, 1);
        ASSERT_EQ(g.tk[i].deadline >= g.tk[i].release, 1);
    }
    scenario_free(&g);
    ASSERT_EQ(scenario_generate(&g, 4, 60, 10, 10, 1), -1);       /* too few rows */
    ASSERT_EQ(scenario_generate(&g, 40, 60, 1000, 10, 1), -1);    /* too many robots */
    ASSERT_EQ(scenario_generate(&g, 40, 60, 0, 10, 1), -1);
    ASSERT_EQ(scenario_generate(NULL, 40, 60, 5, 10, 1), -1);

    /* ---- bottleneck scenario: one gap, still collision free ---- */
    ASSERT_EQ(scenario_bottleneck(&g, 30, 10, 100, 1), 0);
    ASSERT_EQ(g.g.rows, 15);
    ASSERT_EQ(grid_get(&g.g, 7, 15), '.');
    ASSERT_EQ(grid_get(&g.g, 7, 0), '#');
    ASSERT_EQ(sim_run(&g.g, g.rb, g.nr, g.tk, g.nt, &cfg, &m, NULL), 0);
    sane(&m, g.nt);
    scenario_free(&g);
    ASSERT_EQ(scenario_bottleneck(&g, 4, 1, 1, 1), -1);

    /* ---- export ---- */
    ResultRow rows[2];
    memset(rows, 0, sizeof rows);
    snprintf(rows[0].scenario, 32, "normal"); snprintf(rows[0].strategy, 32, "greedy");
    snprintf(rows[1].scenario, 32, "shock");  snprintf(rows[1].strategy, 32, "batch-bnb");
    rows[0].m.completed = 7; rows[1].m.completed = 9;
    ASSERT_EQ(report_write_csv("/tmp/ws_test.csv", rows, 2), 0);
    FILE *fp = fopen("/tmp/ws_test.csv", "r");
    ASSERT_EQ(fp != NULL, 1);
    int lines = 0, ch;
    while ((ch = fgetc(fp)) != EOF) if (ch == '\n') lines++;
    fclose(fp);
    ASSERT_EQ(lines, 3);                                  /* header + 2 rows */
    ASSERT_EQ(report_write_json("/tmp/ws_test.json", rows, 2), 0);
    fp = fopen("/tmp/ws_test.json", "r");
    ASSERT_EQ(fp != NULL, 1);
    ASSERT_EQ(fgetc(fp), '[');
    fclose(fp);
    ASSERT_EQ(report_write_csv("/nonexistent_dir/x.csv", rows, 2), -1);
    ASSERT_EQ(report_write_csv(NULL, rows, 2), -1);
    ASSERT_EQ(report_write_json("/tmp/ws_empty.json", NULL, 0), 0);   /* empty list is fine */
    TEST_SUMMARY();
}