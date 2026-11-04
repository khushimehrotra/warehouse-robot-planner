#ifndef MODEL_H
#define MODEL_H

typedef struct { int id, x, y, battery, payload_cap; } Robot;
typedef struct { int id, shelf_id, dest_id, weight, release, deadline, priority; } Task;
typedef struct { int id, x, y; } Shelf;
typedef struct { int id, x, y, slots; } Station;

#endif