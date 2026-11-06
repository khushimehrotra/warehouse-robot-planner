#include <stdlib.h>
#include "unionfind.h"

int uf_init(UF *u, int n) {
    u->parent = NULL; u->rnk = NULL; u->n = 0;
    if (n <= 0) return -1;
    u->parent = malloc((size_t)n * sizeof(int));
    u->rnk = calloc((size_t)n, sizeof(int));
    if (u->parent == NULL || u->rnk == NULL) {
        free(u->parent); free(u->rnk);
        u->parent = u->rnk = NULL;
        return -3;
    }
    for (int i = 0; i < n; i++) u->parent[i] = i;      /* everyone starts alone */
    u->n = n;
    return 0;
}

/* Path compression: point everything on the way directly at the root. */
int uf_find(UF *u, int x) {
    int root = x;
    while (u->parent[root] != root) root = u->parent[root];
    while (u->parent[x] != root) { int nx = u->parent[x]; u->parent[x] = root; x = nx; }
    return root;
}

/* Union by rank: hang the shorter tree under the taller. Amortized ~O(1). */
int uf_union(UF *u, int a, int b) {
    int ra = uf_find(u, a), rb = uf_find(u, b);
    if (ra == rb) return 0;
    if (u->rnk[ra] < u->rnk[rb]) { int t = ra; ra = rb; rb = t; }
    u->parent[rb] = ra;
    if (u->rnk[ra] == u->rnk[rb]) u->rnk[ra]++;
    return 1;
}

void uf_free(UF *u) {
    free(u->parent); free(u->rnk);
    u->parent = u->rnk = NULL;
    u->n = 0;
}