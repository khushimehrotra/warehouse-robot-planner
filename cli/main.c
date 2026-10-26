#include <stdio.h>
#include "grid.h"

int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : "data/small_grid.txt";

    Grid g;
    int rc = grid_load(path, &g);
    if (rc != 0) {
        printf("Could not load '%s' (error %d)\n", path, rc);
        return 1;
    }

    printf("Loaded %d x %d warehouse:\n", g.rows, g.cols);
    grid_print(&g);

    int shelves = 0, chargers = 0, obstacles = 0;
    for (int r = 0; r < g.rows; r++)
        for (int c = 0; c < g.cols; c++) {
            char ch = grid_get(&g, r, c);
            if (ch == 'S') shelves++;
            else if (ch == 'C') chargers++;
            else if (ch == '#') obstacles++;
        }
    printf("Shelves: %d, Chargers: %d, Obstacles: %d\n", shelves, chargers, obstacles);

    grid_free(&g);
    return 0;
}