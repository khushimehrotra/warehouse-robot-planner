#include <stdlib.h>
#include <string.h>
#include "scenario.h"
#include "robot_io.h"
#include "task_io.h"

static void zero(Scenario *s) { memset(s, 0, sizeof *s); }

void scenario_free(Scenario *s) {
    free(s->g.cells);
    free(s->rb);
    free(s->tk);
    zero(s);
}

int scenario_load(Scenario *s, const char *grid, const char *robots, const char *tasks) {
    if (s == NULL || grid == NULL || robots == NULL || tasks == NULL) return -1;
    zero(s);
    int rc = grid_load(grid, &s->g);
    if (rc != 0) { zero(s); return rc; }
    rc = robots_load(robots, &s->rb, &s->nr);
    if (rc != 0) { scenario_free(s); return rc; }
    rc = tasks_load(tasks, &s->tk, &s->nt);
    if (rc != 0) { scenario_free(s); return rc; }
    return 0;
}

static int alloc_sc(Scenario *s, int rows, int cols, int nr, int nt) {
    zero(s);
    s->g.rows = rows;
    s->g.cols = cols;
    s->g.cells = malloc((size_t)rows * cols);
    s->rb = malloc((size_t)(nr > 0 ? nr : 1) * sizeof(Robot));
    s->tk = malloc((size_t)(nt > 0 ? nt : 1) * sizeof(Task));
    s->nr = nr;
    s->nt = nt;
    if (!s->g.cells || !s->rb || !s->tk) { scenario_free(s); return -3; }
    return 0;
}

static int make_tasks(Scenario *s, unsigned seed) {
    int nsh = 0;
    for (int i = 0; i < s->g.rows * s->g.cols; i++)
        if (s->g.cells[i] == 'S') nsh++;
    if (nsh == 0) return -1;
    srand(seed);                                   /* seeded: runs are repeatable */
    for (int i = 0; i < s->nt; i++) {
        Task *t = &s->tk[i];
        t->id = i + 1;
        t->shelf_id = rand() % nsh;
        t->dest_id = rand() % nsh;
        t->weight = 1 + rand() % 5;
        t->release = i / 2;
        t->deadline = t->release + 400;
        t->priority = 1 + rand() % 3;
    }
    return 0;
}

static void make_robot(Robot *r, int i, int x, int y) {
    r->id = i + 1; r->x = x; r->y = y; r->battery = 100000; r->payload_cap = 10;
}

int scenario_generate(Scenario *s, int rows, int cols, int nr, int nt, unsigned seed) {
    if (s == NULL || rows < 5 || cols < 4 || nr < 1 || nr > 4 * (cols / 2) || nt < 0) return -1;
    int rc = alloc_sc(s, rows, cols, nr, nt);
    if (rc != 0) return rc;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            s->g.cells[r * cols + c] = (r % 3 == 2 && (c % 4 == 1 || c % 4 == 2)) ? 'S' : '.';
    /* robots stand on every second column of rows 0, 1, 3, 4: never on a rack row */
    static const int yrow[4] = {0, 1, 3, 4};
    int per = cols / 2;
    for (int k = 0; k < nr; k++) make_robot(&s->rb[k], k, (k % per) * 2, yrow[k / per]);
    if (make_tasks(s, seed) != 0) { scenario_free(s); return -1; }
    return 0;
}

int scenario_bottleneck(Scenario *s, int cols, int nr, int nt, unsigned seed) {
    if (s == NULL || cols < 8 || nr < 1 || nr > cols / 2 || nt < 0) return -1;
    int rows = 15;
    int rc = alloc_sc(s, rows, cols, nr, nt);
    if (rc != 0) return rc;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++) {
            char ch = '.';
            if (r == 7 && c != cols / 2) ch = '#';                  /* wall with one gap */
            if (r == 12 && (c % 4 == 1 || c % 4 == 2)) ch = 'S';    /* racks below the wall */
            s->g.cells[r * cols + c] = ch;
        }
    for (int k = 0; k < nr; k++) make_robot(&s->rb[k], k, 2 * k, 0);
    if (make_tasks(s, seed) != 0) { scenario_free(s); return -1; }
    return 0;
}

static int clone(const Scenario *in, Scenario *out, int nr) {
    int rc = alloc_sc(out, in->g.rows, in->g.cols, nr, in->nt);
    if (rc != 0) return rc;
    memcpy(out->g.cells, in->g.cells, (size_t)in->g.rows * in->g.cols);
    memcpy(out->rb, in->rb, (size_t)nr * sizeof(Robot));
    memcpy(out->tk, in->tk, (size_t)in->nt * sizeof(Task));
    return 0;
}

int scenario_shock_fleet(const Scenario *in, Scenario *out, int lose) {
    if (in == NULL || out == NULL || lose < 0 || lose >= in->nr) return -1;
    return clone(in, out, in->nr - lose);
}

int scenario_shock_block(const Scenario *in, Scenario *out) {
    if (in == NULL || out == NULL) return -1;
    int rc = clone(in, out, in->nr);
    if (rc != 0) return rc;
    int r = out->g.rows / 2, cols = out->g.cols;
    for (int c = cols / 6; c < cols - cols / 6; c++)
        if (out->g.cells[r * cols + c] == '.') out->g.cells[r * cols + c] = '#';
    return 0;
}