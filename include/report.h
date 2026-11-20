#ifndef REPORT_H
#define REPORT_H
#include "sim.h"

typedef struct {
    char scenario[32], strategy[32];   /* fixed labels: no characters that need escaping */
    int nr, nt;
    SimMetrics m;
    double ms;                         /* wall-clock time of the run */
    long mem_kb;                       /* process peak memory so far (it never goes down) */
} ResultRow;

/* Return 0 ok, -1 bad arguments or cannot open the file, -2 write error. */
int report_write_csv(const char *path, const ResultRow *rows, int n);
int report_write_json(const char *path, const ResultRow *rows, int n);

#endif