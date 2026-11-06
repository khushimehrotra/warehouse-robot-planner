#include <stdio.h>
#include <stdlib.h>
#include "grid.h"
#include "gridpath.h"

int main(int argc, char *argv[]) {
    const char *file = (argc > 1) ? argv[1] : "data/small_grid.txt";

    Grid g;
    int rc = grid_load(file, &g);
    if (rc != 0) {
        printf("Could not load '%s' (error %d)\n", file, rc);
        return 1;
    }
    printf("Loaded %d x %d warehouse:\n", g.rows, g.cols);
    grid_print(&g);

    int sr, sc, tr, tc;
    if (grid_find(&g, 'C', &sr, &sc) != 0 || grid_find(&g, 'S', &tr, &tc) != 0) {
        printf("Map needs at least one charger (C) and one shelf (S).\n");
        grid_free(&g);
        return 1;
    }

    int *route = malloc((size_t)g.rows * g.cols * sizeof(int));
    if (route == NULL) { grid_free(&g); return 1; }
    int len;
    rc = grid_shortest_path(&g, sr, sc, tr, tc, route, g.rows * g.cols, &len);
    if (rc == 0) {
        printf("\nRoute from charger (%d,%d) to shelf (%d,%d): %d steps\n",
               sr, sc, tr, tc, len - 1);
        grid_print_path(&g, route, len);
    } else {
        printf("\nNo route found (code %d)\n", rc);
    }
    free(route);
    grid_free(&g);
    return 0;
}