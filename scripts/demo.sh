#!/bin/sh
# Demonstration: three scenarios, two strategies each, exported to docs/
set -e
make planner_fast
./planner_fast normal
./planner_fast shock
./planner_fast stress 100 10000
./planner_fast bench
echo "Results are in docs/results_*.csv, docs/results_*.json and docs/bench_w7.csv"
