#include <stdio.h>
#include <stdlib.h>
#include "task_io.h"

int tasks_load(const char *path, Task **out, int *count) {
    *out = NULL;
    *count = 0;

    FILE *f = fopen(path, "r");
    if (f == NULL) return -1;

    int n;
    if (fscanf(f, "%d", &n) != 1 || n < 0 || n > 10000000) {
        fclose(f);
        return -2;
    }

    Task *t = malloc((size_t)(n > 0 ? n : 1) * sizeof(Task));
    if (t == NULL) { fclose(f); return -3; }

    for (int i = 0; i < n; i++) {
        if (fscanf(f, "%d %d %d %d %d %d %d",
                   &t[i].id, &t[i].shelf_id, &t[i].dest_id, &t[i].weight,
                   &t[i].release, &t[i].deadline, &t[i].priority) != 7) {
            free(t); fclose(f);
            return -4;
        }
        if (t[i].weight <= 0 || t[i].deadline < t[i].release) {
            free(t); fclose(f);
            return -5;
        }
    }
    fclose(f);
    *out = t;
    *count = n;
    return 0;
}