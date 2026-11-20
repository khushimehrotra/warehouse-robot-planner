#include <stdio.h>
#include "report.h"

int report_write_csv(const char *path, const ResultRow *rows, int n) {
    if (path == NULL || n < 0 || (n > 0 && rows == NULL)) return -1;
    FILE *f = fopen(path, "w");
    if (f == NULL) return -1;
    fprintf(f, "scenario,strategy,robots,tasks,completed,failed,unserved,makespan,"
               "deadline_misses,avg_latency,max_latency,total_distance,total_wait,"
               "utilization,collisions,plan_calls,plan_failures,runtime_ms,peak_mem_kb\n");
    for (int i = 0; i < n; i++) {
        const ResultRow *r = &rows[i];
        const SimMetrics *m = &r->m;
        fprintf(f, "%s,%s,%d,%d,%d,%d,%d,%d,%d,%.3f,%d,%lld,%lld,%.4f,%d,%lld,%lld,%.3f,%ld\n",
                r->scenario, r->strategy, r->nr, r->nt, m->completed, m->failed, m->unserved,
                m->makespan, m->deadline_misses, m->avg_latency, m->max_latency,
                m->total_distance, m->total_wait, m->utilization, m->collisions,
                m->plan_calls, m->plan_failures, r->ms, r->mem_kb);
    }
    return fclose(f) == 0 ? 0 : -2;
}

int report_write_json(const char *path, const ResultRow *rows, int n) {
    if (path == NULL || n < 0 || (n > 0 && rows == NULL)) return -1;
    FILE *f = fopen(path, "w");
    if (f == NULL) return -1;
    fprintf(f, "[\n");
    for (int i = 0; i < n; i++) {
        const ResultRow *r = &rows[i];
        const SimMetrics *m = &r->m;
        fprintf(f, "  {\"scenario\": \"%s\", \"strategy\": \"%s\", \"robots\": %d, \"tasks\": %d,\n"
                   "   \"completed\": %d, \"failed\": %d, \"unserved\": %d, \"makespan\": %d,\n"
                   "   \"deadline_misses\": %d, \"avg_latency\": %.3f, \"max_latency\": %d,\n"
                   "   \"total_distance\": %lld, \"total_wait\": %lld, \"utilization\": %.4f,\n"
                   "   \"collisions\": %d, \"plan_calls\": %lld, \"plan_failures\": %lld,\n"
                   "   \"runtime_ms\": %.3f, \"peak_mem_kb\": %ld}%s\n",
                r->scenario, r->strategy, r->nr, r->nt, m->completed, m->failed, m->unserved,
                m->makespan, m->deadline_misses, m->avg_latency, m->max_latency,
                m->total_distance, m->total_wait, m->utilization, m->collisions,
                m->plan_calls, m->plan_failures, r->ms, r->mem_kb, i + 1 < n ? "," : "");
    }
    fprintf(f, "]\n");
    return fclose(f) == 0 ? 0 : -2;
}