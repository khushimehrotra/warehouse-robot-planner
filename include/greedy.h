#ifndef GREEDY_H
#define GREEDY_H

typedef struct { int id, start, finish; } Interval;

/* Most non-overlapping intervals.
   chosen[] must have room for n ints; it receives the chosen ids.
   Return: 0 ok, -1 bad arguments, -2 interval with finish <= start, -3 out of memory. */
int activity_select(const Interval *iv, int n, int *chosen, int *count);

/* Fewest rooms (docks) so no two overlapping intervals share one.
   room_of[i] = room of iv[i] (by position in the input, not by id).
   room_of must have room for n ints. *rooms = number of rooms used.
   Return codes as above. */
int interval_partition(const Interval *iv, int n, int *room_of, int *rooms);

#endif