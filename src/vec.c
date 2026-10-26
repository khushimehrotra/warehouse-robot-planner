/* src/vec.c */
#include <stdlib.h>
#include "vec.h"

void vec_init(Vec *v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

int vec_push(Vec *v, int value) {
    if (v->size == v->capacity) {                 /* full? double the space */
        int new_cap = (v->capacity == 0) ? 4 : v->capacity * 2;
        int *p = realloc(v->data, new_cap * sizeof(int));
        if (p == NULL) return -1;                 /* out of memory */
        v->data = p;
        v->capacity = new_cap;
    }
    v->data[v->size] = value;
    v->size++;
    return 0;
}

int vec_get(const Vec *v, int i) {
    return v->data[i];
}

void vec_free(Vec *v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}