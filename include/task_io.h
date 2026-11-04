#ifndef TASK_IO_H
#define TASK_IO_H
#include "model.h"

/* Return codes: 0 ok, -1 cannot open file, -2 bad count,
   -3 out of memory, -4 bad row, -5 invalid values.
   On success the caller must free(*out). */
int tasks_load(const char *path, Task **out, int *count);

#endif