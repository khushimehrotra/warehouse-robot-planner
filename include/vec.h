/* include/vec.h */
#ifndef VEC_H
#define VEC_H

typedef struct {
    int *data;      /* the actual numbers */
    int size;       /* how many are stored */
    int capacity;   /* how many fit before we must grow */
} Vec;

void vec_init(Vec *v);
int  vec_push(Vec *v, int value);   /* 0 = ok, -1 = out of memory */
int  vec_get(const Vec *v, int i);  /* caller must keep 0 <= i < size */
void vec_free(Vec *v);

#endif