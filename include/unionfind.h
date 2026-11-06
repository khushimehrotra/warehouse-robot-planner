#ifndef UNIONFIND_H
#define UNIONFIND_H

typedef struct { int *parent; int *rnk; int n; } UF;

int  uf_init(UF *u, int n);            /* 0 ok, -1 bad n, -3 out of memory */
int  uf_find(UF *u, int x);            /* x must be in 0..n-1 */
int  uf_union(UF *u, int a, int b);    /* 1 = merged two sets, 0 = already together */
void uf_free(UF *u);

#endif