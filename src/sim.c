#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "sim.h"
#include "minheap.h"
#include "vec.h"
#include "bnb.h"

enum { R_IDLE = 0, R_NEEDPLAN, R_MOVING, R_DWELL };
enum { T_NEW = 0, T_ACTIVE, T_DONE, T_FAILED };
#define INFEASIBLE 100000

static const int DR[4] = {1, -1, 0, 0};
static const int DC[4] = {0, 0, 1, -1};

typedef struct {
    int cell, home, state, task, leg, dwell, len, step;
    int path[SIM_WIN];          /* path[step] is the current cell */
} Rob;                          /* leg 0 = to shelf, 1 = to destination, 2 = home */

typedef struct { int rel, idx; } RelItem;

typedef struct {
    const Grid *g;
    const Robot *robots;
    int rows, cols, cells, nr, nsh, slack, avoid;
    int *shelf_cell;
    Rob *rob;
    char *tstate;
    unsigned short *resv;       /* resv[(t % WIN)*cells + cell] = robot id + 1 */
    int *park_owner, *park_from;/* a parked robot blocks its cell for t >= park_from */
    int *hdist, *queue;         /* BFS distance to the goal cells */
    unsigned *seen;             /* stamp per (dt, cell) state, avoids clearing */
    int *par;                   /* previous cell of each state */
    unsigned stamp;
    MinHeap open, pending;
    long long plan_calls, plan_fail;
} Sim;

void sim_default_config(SimConfig *c) {
    c->assign = SIM_ASSIGN_GREEDY;
    c->avoid_collisions = 1;
    c->max_ticks = 100000;
    c->slack = 40;
    c->log = NULL;
}

static int passable(const Sim *s, int r, int c) {
    return grid_is_walkable(s->g, r, c) && grid_get(s->g, r, c) != 'S';
}

/* Who stands in 'cell' at tick t? (robot id + 1, or 0.) */
static int who(const Sim *s, int cell, int t) {
    if (!s->avoid) return 0;
    int w = s->resv[(size_t)(t % SIM_WIN) * s->cells + cell];
    if (w) return w;
    if (s->park_owner[cell] && t >= s->park_from[cell]) return s->park_owner[cell];
    return 0;
}

/* Multi-source BFS: hdist[cell] = steps to the nearest goal cell, -1 if none reachable. */
static void bfs_goals(Sim *s, const int *goals, int ng) {
    for (int i = 0; i < s->cells; i++) s->hdist[i] = -1;
    int head = 0, tail = 0;
    for (int i = 0; i < ng; i++)
        if (s->hdist[goals[i]] < 0) { s->hdist[goals[i]] = 0; s->queue[tail++] = goals[i]; }
    while (head < tail) {
        int u = s->queue[head++], r = u / s->cols, c = u % s->cols;
        for (int k = 0; k < 4; k++) {
            int vr = r + DR[k], vc = c + DC[k];
            if (!passable(s, vr, vc)) continue;
            int v = vr * s->cols + vc;
            if (s->hdist[v] < 0) { s->hdist[v] = s->hdist[u] + 1; s->queue[tail++] = v; }
        }
    }
}

static int shelf_goals(const Sim *s, int shelf, int *out) {      /* aisle cells next to a rack */
    int sc = s->shelf_cell[shelf], r = sc / s->cols, c = sc % s->cols, n = 0;
    for (int k = 0; k < 4; k++)
        if (passable(s, r + DR[k], c + DC[k])) out[n++] = (r + DR[k]) * s->cols + (c + DC[k]);
    return n;
}

/* Arriving at 'goal' at tick t and staying: nobody else may be booked there later. */
static int goal_free_after(const Sim *s, int goal, int t, int t0, int me) {
    if (!s->avoid) return 1;
    if (s->park_owner[goal] != 0 && s->park_owner[goal] != me) return 0;
    for (int tau = t + 1; tau < t0 + SIM_WIN; tau++) {
        int w = s->resv[(size_t)(tau % SIM_WIN) * s->cells + goal];
        if (w != 0 && w != me) return 0;
    }
    return 1;
}

/* Space-time A*. Return 0 = path stored in rob->path, 1 = blocked right now (retry later),
   -2 = impossible (unreachable or longer than the window), -3 = out of memory. */
static int plan_leg(Sim *s, int id, const int *goals, int ng, int t0) {
    Rob *rb = &s->rob[id];
    int start = rb->cell, cells = s->cells, me = id + 1;
    s->plan_calls++;
    bfs_goals(s, goals, ng);
    int h0 = s->hdist[start];
    if (h0 < 0 || h0 > SIM_WIN - 1) return -2;
    int hmax = h0 + s->slack;
    if (hmax > SIM_WIN - 1) hmax = SIM_WIN - 1;

    s->stamp++;
    s->open.size = 0;
    s->seen[start] = s->stamp;                  /* state id = dt * cells + cell; dt = 0 here */
    s->par[start] = -1;
    if (heap_push(&s->open, h0, start) != 0) return -3;

    int found = -1;
    HeapItem it;
    while (heap_pop(&s->open, &it) == 0) {
        int dt = it.value / cells, u = it.value % cells, t = t0 + dt;
        if (s->hdist[u] == 0 && goal_free_after(s, u, t, t0, me)) { found = it.value; break; }
        if (dt >= hmax) continue;
        int ur = u / s->cols, uc = u % s->cols;
        for (int k = 0; k < 5; k++) {           /* k = 4 means "wait here" */
            int v = u;
            if (k < 4) {
                int vr = ur + DR[k], vc = uc + DC[k];
                if (!passable(s, vr, vc)) continue;
                v = vr * s->cols + vc;
            }
            int ndt = dt + 1;
            if (s->hdist[v] < 0 || ndt + s->hdist[v] > hmax) continue;
            size_t ns = (size_t)ndt * cells + v;
            if (s->seen[ns] == s->stamp) continue;
            int w = who(s, v, t + 1);           /* vertex conflict */
            if (w != 0 && w != me) continue;
            if (v != u) {                       /* swap conflict: someone goes v -> u while I go u -> v */
                int w2 = who(s, u, t + 1);
                if (w2 != 0 && w2 != me && who(s, v, t) == w2) continue;
            }
            s->seen[ns] = s->stamp;
            s->par[ns] = u;
            if (heap_push(&s->open, ndt + s->hdist[v], (int)ns) != 0) return -3;
        }
    }
    if (found < 0) return 1;

    int d = found / cells, cell = found % cells;
    for (int L = d; L >= 0; L--) {              /* walk the parents back */
        rb->path[L] = cell;
        if (L > 0) cell = s->par[(size_t)L * cells + cell];
    }
    rb->len = d;
    rb->step = 0;
    return 0;
}

static void arrive(Rob *rb) {
    if (rb->leg == 2) rb->state = R_IDLE;
    else { rb->state = R_DWELL; rb->dwell = 1; }
}

/* Write the planned path into the reservation table and park at its end. */
static void commit(Sim *s, int id, int t0) {
    Rob *rb = &s->rob[id];
    int goal = rb->path[rb->len];
    if (s->park_owner[rb->cell] == id + 1) s->park_owner[rb->cell] = 0;
    for (int d = 0; d <= rb->len; d++)
        s->resv[(size_t)((t0 + d) % SIM_WIN) * s->cells + rb->path[d]] = (unsigned short)(id + 1);
    s->park_owner[goal] = id + 1;
    s->park_from[goal] = t0 + rb->len;
    if (rb->len == 0) arrive(rb); else rb->state = R_MOVING;
}

static void start_task(Sim *s, int r, int ti) {
    Rob *rb = &s->rob[r];
    rb->task = ti;
    rb->leg = 0;
    rb->state = R_NEEDPLAN;
    s->tstate[ti] = T_ACTIVE;
}

/* ---------- strategy 1: greedy, most urgent task first, nearest idle robot ---------- */
static int assign_greedy(Sim *s, const Task *tasks) {
    int idle = 0, rc = 0;
    for (int r = 0; r < s->nr; r++) if (s->rob[r].state == R_IDLE) idle++;
    if (idle == 0) return 0;
    Vec skip;
    vec_init(&skip);
    HeapItem it;
    while (idle > 0 && heap_pop(&s->pending, &it) == 0) {
        int ti = it.value, sc = s->shelf_cell[tasks[ti].shelf_id];
        int best = -1, bd = INT_MAX;
        for (int r = 0; r < s->nr; r++) {
            const Rob *rb = &s->rob[r];
            if (rb->state != R_IDLE || s->robots[r].payload_cap < tasks[ti].weight) continue;
            int d = abs(rb->cell / s->cols - sc / s->cols) + abs(rb->cell % s->cols - sc % s->cols);
            if (d < bd) { bd = d; best = r; }        /* Manhattan distance: a cheap estimate */
        }
        if (best < 0) {                              /* no idle robot can carry it: try later */
            if (vec_push(&skip, ti) != 0) { rc = -3; break; }
            continue;
        }
        start_task(s, best, ti);
        idle--;
    }
    for (int i = 0; i < skip.size && rc == 0; i++)
        if (heap_push(&s->pending, tasks[skip.data[i]].deadline, skip.data[i]) != 0) rc = -3;
    vec_free(&skip);
    return rc;
}

/* ---------- strategy 2: exact assignment of up to 8 idle robots to the 8 most urgent tasks.
   Rows = idle robots, columns = tasks (padded with free dummy tasks so the matrix is square). */
static int assign_batch(Sim *s, const Task *tasks) {
    int idle[8], ni = 0, pick[8], np = 0, rc = 0;
    for (int r = 0; r < s->nr && ni < 8; r++)
        if (s->rob[r].state == R_IDLE) idle[ni++] = r;
    if (ni == 0) return 0;

    Vec skip;
    vec_init(&skip);
    HeapItem it;
    while (np < ni && heap_pop(&s->pending, &it) == 0) {
        int ti = it.value, ok = 0;
        for (int i = 0; i < ni; i++)
            if (s->robots[idle[i]].payload_cap >= tasks[ti].weight) ok = 1;
        if (!ok) {
            if (vec_push(&skip, ti) != 0) { rc = -3; break; }
            continue;
        }
        pick[np++] = ti;
    }

    char used[8] = {0};
    if (rc == 0 && np > 0) {
        int c[64];
        for (int i = 0; i < ni; i++)
            for (int j = 0; j < ni; j++) {
                int cost = 0;                                  /* dummy column */
                if (j < np) {
                    const Task *tk = &tasks[pick[j]];
                    int sc = s->shelf_cell[tk->shelf_id];
                    const Rob *rb = &s->rob[idle[i]];
                    cost = (s->robots[idle[i]].payload_cap >= tk->weight)
                         ? abs(rb->cell / s->cols - sc / s->cols) + abs(rb->cell % s->cols - sc % s->cols)
                         : INFEASIBLE;
                }
                c[i * ni + j] = cost;
            }
        int as[BNB_MAX_N];
        long long tot;
        int brc = assign_bnb(c, ni, BNB_LC, as, &tot, NULL);   /* Week 5 */
        if (brc == -3) rc = -3;
        else if (brc == 0)
            for (int i = 0; i < ni; i++) {
                int j = as[i];
                if (j < np && c[i * ni + j] < INFEASIBLE) { start_task(s, idle[i], pick[j]); used[j] = 1; }
            }
    }
    for (int j = 0; j < np && rc == 0; j++)                    /* unassigned tasks go back */
        if (!used[j] && heap_push(&s->pending, tasks[pick[j]].deadline, pick[j]) != 0) rc = -3;
    for (int i = 0; i < skip.size && rc == 0; i++)
        if (heap_push(&s->pending, tasks[skip.data[i]].deadline, skip.data[i]) != 0) rc = -3;
    vec_free(&skip);
    return rc;
}

static int cmp_rel(const void *a, const void *b) {
    const RelItem *x = a, *y = b;
    if (x->rel != y->rel) return (x->rel > y->rel) - (x->rel < y->rel);
    return (x->idx > y->idx) - (x->idx < y->idx);
}

int sim_run(const Grid *g, const Robot *robots, int nr, const Task *tasks, int nt,
            const SimConfig *cfg, SimMetrics *out, int *finish_out) {
    SimConfig def;
    Sim s;
    RelItem *ord = NULL;
    int *finish = NULL, *prev = NULL, *occa = NULL, *occb = NULL;
    int rc = 0, t, nextrel = 0, finished = 0, failed = 0, completed = 0, misses = 0;
    int last_finish = 0, max_lat = 0, collisions = 0, ticks = 0, maxcap = 0, nsh = 0;
    long long busy = 0, dist = 0, wait = 0;
    double lat_sum = 0;

    if (out == NULL) return -1;
    memset(out, 0, sizeof *out);
    if (cfg == NULL) { sim_default_config(&def); cfg = &def; }
    if (g == NULL || g->rows <= 0 || g->cols <= 0 || robots == NULL || nr < 1 || nr > 60000 ||
        nt < 0 || (nt > 0 && tasks == NULL) || cfg->max_ticks < 1 || cfg->slack < 0 ||
        (int)cfg->assign < 0 || (int)cfg->assign > 1) return -1;
    if ((size_t)g->rows * g->cols > 65536) return -1;

    memset(&s, 0, sizeof s);
    s.g = g; s.robots = robots; s.rows = g->rows; s.cols = g->cols;
    s.cells = g->rows * g->cols; s.nr = nr; s.slack = cfg->slack; s.avoid = cfg->avoid_collisions;
    int cells = s.cells;
    for (int i = 0; i < cells; i++) if (g->cells[i] == 'S') nsh++;
    s.nsh = nsh;
    int ntt = nt > 0 ? nt : 1;

    s.rob = calloc((size_t)nr, sizeof(Rob));
    s.tstate = calloc((size_t)ntt, 1);
    finish = malloc((size_t)ntt * sizeof(int));
    ord = malloc((size_t)ntt * sizeof(RelItem));
    s.resv = calloc((size_t)SIM_WIN * cells, sizeof(unsigned short));
    s.park_owner = calloc((size_t)cells, sizeof(int));
    s.park_from = calloc((size_t)cells, sizeof(int));
    s.hdist = malloc((size_t)cells * sizeof(int));
    s.queue = malloc((size_t)cells * sizeof(int));
    s.seen = calloc((size_t)cells * SIM_WIN, sizeof(unsigned));
    s.par = malloc((size_t)cells * SIM_WIN * sizeof(int));
    s.shelf_cell = malloc((size_t)(nsh > 0 ? nsh : 1) * sizeof(int));
    prev = malloc((size_t)nr * sizeof(int));
    occa = malloc((size_t)cells * sizeof(int));
    occb = malloc((size_t)cells * sizeof(int));
    if (!s.rob || !s.tstate || !finish || !ord || !s.resv || !s.park_owner || !s.park_from ||
        !s.hdist || !s.queue || !s.seen || !s.par || !s.shelf_cell || !prev || !occa || !occb ||
        heap_init(&s.open, 256) != 0 || heap_init(&s.pending, 64) != 0) { rc = -3; goto done; }

    for (int i = 0; i < cells; i++) occa[i] = occb[i] = -1;
    for (int i = 0, k = 0; i < cells; i++) if (g->cells[i] == 'S') s.shelf_cell[k++] = i;

    for (int r = 0; r < nr; r++) {                         /* validate and place the robots */
        int x = robots[r].x, y = robots[r].y;
        if (!grid_is_walkable(g, y, x) || grid_get(g, y, x) == 'S' || occa[y * s.cols + x] >= 0) { rc = -1; goto done; }
        int cell = y * s.cols + x;
        occa[cell] = r;
        s.rob[r].cell = s.rob[r].home = cell;
        s.rob[r].state = R_IDLE;
        s.rob[r].task = -1;
        s.park_owner[cell] = r + 1;                        /* parked from tick 0 */
        s.park_from[cell] = 0;
        if (robots[r].payload_cap > maxcap) maxcap = robots[r].payload_cap;
    }
    for (int r = 0; r < nr; r++) occa[s.rob[r].cell] = -1;

    for (int i = 0; i < nt; i++) {                         /* validate the tasks */
        const Task *tk = &tasks[i];
        if (tk->shelf_id < 0 || tk->shelf_id >= nsh || tk->dest_id < 0 || tk->dest_id >= nsh ||
            tk->weight <= 0 || tk->release < 0 || tk->deadline < tk->release) { rc = -1; goto done; }
        finish[i] = -1;
        ord[i].rel = tk->release;
        ord[i].idx = i;
        if (tk->weight > maxcap) { s.tstate[i] = T_FAILED; failed++; finished++; }  /* nobody can lift it */
    }
    qsort(ord, (size_t)nt, sizeof(RelItem), cmp_rel);

    for (t = 0; t < cfg->max_ticks; t++) {
        ticks = t + 1;

        /* 0. the slot of tick t-1 is now in the past: reuse it for tick t+WIN-1 */
        if (t > 0)
            memset(s.resv + (size_t)((t - 1) % SIM_WIN) * cells, 0, (size_t)cells * sizeof(unsigned short));

        /* 1. release tasks */
        while (nextrel < nt && ord[nextrel].rel <= t) {
            int ti = ord[nextrel++].idx;
            if (s.tstate[ti] == T_NEW && heap_push(&s.pending, tasks[ti].deadline, ti) != 0) { rc = -3; goto done; }
        }

        /* 2. assign pending tasks to idle robots */
        if (!heap_empty(&s.pending)) {
            rc = (cfg->assign == SIM_ASSIGN_GREEDY) ? assign_greedy(&s, tasks) : assign_batch(&s, tasks);
            if (rc != 0) goto done;
        }

        /* 3. plan a route for every robot that needs one (lower id = higher priority) */
        for (int r = 0; r < nr; r++) {
            Rob *rb = &s.rob[r];
            if (rb->state != R_NEEDPLAN) continue;
            int goals[4], ng;
            if (rb->leg == 0) ng = shelf_goals(&s, tasks[rb->task].shelf_id, goals);
            else if (rb->leg == 1) ng = shelf_goals(&s, tasks[rb->task].dest_id, goals);
            else { goals[0] = rb->home; ng = 1; }
            int pr = plan_leg(&s, r, goals, ng, t);
            if (pr == -3) { rc = -3; goto done; }
            if (pr == 0) commit(&s, r, t);
            else if (pr == 1) s.plan_fail++;              /* blocked: wait and retry next tick */
            else if (rb->leg < 2) {                       /* impossible leg: give the task up */
                s.tstate[rb->task] = T_FAILED;
                failed++; finished++;
                rb->task = -1;
                rb->leg = 2;
                rb->state = (rb->cell == rb->home) ? R_IDLE : R_NEEDPLAN;
            } else rb->state = R_IDLE;
        }

        /* 4. metrics and event log for this tick */
        for (int r = 0; r < nr; r++) {
            const Rob *rb = &s.rob[r];
            char ch;
            if (rb->task >= 0) busy++;
            switch (rb->state) {
            case R_IDLE: ch = 'I'; break;
            case R_NEEDPLAN: ch = 'W'; if (rb->task >= 0) wait++; break;
            case R_MOVING:
                if (rb->path[rb->step + 1] != rb->path[rb->step]) ch = 'M';
                else { ch = 'W'; if (rb->task >= 0) wait++; }
                break;
            default: ch = (rb->leg == 0) ? 'P' : 'D'; break;
            }
            if (cfg->log && r == cfg->log->robot && cfg->log->len < cfg->log->cap)
                cfg->log->buf[cfg->log->len++] = ch;
        }

        /* 5. advance every robot from tick t to t+1 */
        for (int r = 0; r < nr; r++) prev[r] = s.rob[r].cell;
        for (int r = 0; r < nr; r++) {
            Rob *rb = &s.rob[r];
            if (rb->state == R_MOVING) {
                rb->step++;
                rb->cell = rb->path[rb->step];
                if (rb->cell != prev[r]) dist++;
                if (rb->step == rb->len) arrive(rb);
            } else if (rb->state == R_DWELL && --rb->dwell <= 0) {
                if (rb->leg == 0) { rb->leg = 1; rb->state = R_NEEDPLAN; }
                else {                                    /* drop finished: the task is done */
                    int ti = rb->task, lat = (t + 1) - tasks[ti].release;
                    s.tstate[ti] = T_DONE;
                    finish[ti] = t + 1;
                    completed++; finished++;
                    lat_sum += lat;
                    if (lat > max_lat) max_lat = lat;
                    if (t + 1 > tasks[ti].deadline) misses++;
                    if (t + 1 > last_finish) last_finish = t + 1;
                    rb->task = -1;
                    rb->leg = 2;
                    rb->state = (rb->cell == rb->home) ? R_IDLE : R_NEEDPLAN;
                }
            }
        }

        /* 6. collision checker: two robots in one cell, or two robots swapping cells */
        for (int r = 0; r < nr; r++) {
            if (occa[s.rob[r].cell] >= 0) collisions++; else occa[s.rob[r].cell] = r;
            occb[prev[r]] = r;
        }
        for (int r = 0; r < nr; r++) {
            int cur = s.rob[r].cell;
            if (prev[r] != cur) {
                int o = occb[cur];
                if (o >= 0 && o != r && prev[o] != s.rob[o].cell && s.rob[o].cell == prev[r] && r < o)
                    collisions++;
            }
        }
        for (int r = 0; r < nr; r++) { occa[s.rob[r].cell] = -1; occb[prev[r]] = -1; }

        /* 7. stop when every task is finished or failed and all robots are home */
        if (finished == nt) {
            int all_idle = 1;
            for (int r = 0; r < nr && all_idle; r++) if (s.rob[r].state != R_IDLE) all_idle = 0;
            if (all_idle) break;
        }
    }

    out->ticks = ticks;
    out->makespan = last_finish;
    out->completed = completed;
    out->failed = failed;
    out->unserved = nt - completed - failed;
    out->deadline_misses = misses;
    out->avg_latency = completed > 0 ? lat_sum / completed : 0.0;
    out->max_latency = max_lat;
    out->total_distance = dist;
    out->total_wait = wait;
    out->utilization = ticks > 0 ? (double)busy / ((double)nr * ticks) : 0.0;
    out->collisions = collisions;
    out->plan_calls = s.plan_calls;
    out->plan_failures = s.plan_fail;
    if (finish_out != NULL && nt > 0) memcpy(finish_out, finish, (size_t)nt * sizeof(int));

done:
    free(s.rob); free(s.tstate); free(finish); free(ord); free(s.resv);
    free(s.park_owner); free(s.park_from); free(s.hdist); free(s.queue);
    free(s.seen); free(s.par); free(s.shelf_cell); free(prev); free(occa); free(occb);
    heap_free(&s.open);
    heap_free(&s.pending);
    return rc;
}