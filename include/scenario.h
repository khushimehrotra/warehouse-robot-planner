#ifndef SCENARIO_H
#define SCENARIO_H
#include "grid.h"
#include "model.h"

typedef struct { Grid g; Robot *rb; Task *tk; int nr, nt; } Scenario;

/* Return 0 ok, -1 bad arguments, -3 out of memory, or the loader's error code. */
int scenario_load(Scenario *s, const char *grid, const char *robots, const char *tasks);

/* Racks in pairs on every third row, open aisles elsewhere.
   Needs rows >= 5, cols >= 4, 1 <= nr <= 4*(cols/2), nt >= 0.
   Tasks are released 2 per tick, deadline = release + 400. */
int scenario_generate(Scenario *s, int rows, int cols, int nr, int nt, unsigned seed);

/* Adversarial: a 15-row warehouse split by a wall with ONE gap. Robots start in the
   top half, all racks are in the bottom half. Needs cols >= 8, 1 <= nr <= cols/2. */
int scenario_bottleneck(Scenario *s, int cols, int nr, int nt, unsigned seed);

/* Fleet shock: copy of 'in' without its last 'lose' robots (0 <= lose < in->nr). */
int scenario_shock_fleet(const Scenario *in, Scenario *out, int lose);

/* Aisle shock: copy of 'in' with the middle row walled off (floor cells only,
   the middle two thirds of its width). */
int scenario_shock_block(const Scenario *in, Scenario *out);

void scenario_free(Scenario *s);   /* safe on a zeroed or already freed Scenario */

#endif