#include <stdio.h>
#include <stdlib.h>
#include "robot_io.h"

int robots_load(const char *path, Robot **out, int *count) {
    *out = NULL;
    *count = 0;

    FILE *f = fopen(path, "r");
    if (f == NULL) return -1;

    int n;
    if (fscanf(f, "%d", &n) != 1 || n < 0 || n > 1000000) {
        fclose(f);
        return -2;
    }

    Robot *r = malloc((size_t)(n > 0 ? n : 1) * sizeof(Robot));
    if (r == NULL) { fclose(f); return -3; }

    for (int i = 0; i < n; i++) {
        if (fscanf(f, "%d %d %d %d %d",
                   &r[i].id, &r[i].x, &r[i].y,
                   &r[i].battery, &r[i].payload_cap) != 5) {
            free(r); fclose(f);
            return -4;
        }
        if (r[i].battery < 0 || r[i].payload_cap <= 0 ||
            r[i].x < 0 || r[i].y < 0) {
            free(r); fclose(f);
            return -5;
        }
    }
    fclose(f);
    *out = r;
    *count = n;
    return 0;
}