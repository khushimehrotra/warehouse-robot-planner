#ifndef SORT_H
#define SORT_H

int  merge_sort(int *a, int n);   /* 0 ok, -1 out of memory */
void quick_sort(int *a, int n);   /* random pivot */
int  bin_search_iter(const int *a, int n, int key);              /* index or -1 */
int  bin_search_rec(const int *a, int lo, int hi, int key);      /* index or -1 */

#endif