#include <stdlib.h>
#include "sort.h"

/* Recurrence: T(n) = 2T(n/2) + O(n)  ->  O(n log n) */
static void merge(int *a, int *tmp, int lo, int mid, int hi) {
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi)
        tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];   /* <= keeps it stable */
    while (i <= mid) tmp[k++] = a[i++];
    while (j <= hi)  tmp[k++] = a[j++];
    for (k = lo; k <= hi; k++) a[k] = tmp[k];
}

static void msort(int *a, int *tmp, int lo, int hi) {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    msort(a, tmp, lo, mid);
    msort(a, tmp, mid + 1, hi);
    merge(a, tmp, lo, mid, hi);
}

int merge_sort(int *a, int n) {
    if (n <= 1) return 0;
    int *tmp = malloc((size_t)n * sizeof(int));
    if (tmp == NULL) return -1;
    msort(a, tmp, 0, n - 1);
    free(tmp);
    return 0;
}

/* Average T(n) = 2T(n/2) + O(n) -> O(n log n); worst case O(n^2) */
static void swap_int(int *x, int *y) { int t = *x; *x = *y; *y = t; }

static void qsort_rec(int *a, int lo, int hi) {
    while (lo < hi) {
        int p = lo + rand() % (hi - lo + 1);   /* random pivot */
        swap_int(&a[p], &a[hi]);
        int pivot = a[hi], i = lo;
        for (int j = lo; j < hi; j++)
            if (a[j] < pivot) swap_int(&a[i++], &a[j]);
        swap_int(&a[i], &a[hi]);
        /* recurse on the smaller side, loop on the larger: keeps stack small */
        if (i - lo < hi - i) { qsort_rec(a, lo, i - 1); lo = i + 1; }
        else                 { qsort_rec(a, i + 1, hi); hi = i - 1; }
    }
}

void quick_sort(int *a, int n) {
    if (n > 1) qsort_rec(a, 0, n - 1);
}

int bin_search_iter(const int *a, int n, int key) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;      /* avoids overflow of (lo+hi)/2 */
        if (a[mid] == key) return mid;
        if (a[mid] < key) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

int bin_search_rec(const int *a, int lo, int hi, int key) {
    if (lo > hi) return -1;
    int mid = lo + (hi - lo) / 2;
    if (a[mid] == key) return mid;
    if (a[mid] < key) return bin_search_rec(a, mid + 1, hi, key);
    return bin_search_rec(a, lo, mid - 1, key);
}