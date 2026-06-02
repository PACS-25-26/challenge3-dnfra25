#!/bin/bash

echo "np,time" > results.csv

export OMP_NUM_THREADS=1

SOLVER=../solver

for np in 1 2 4
do
    echo "Running with $np MPI ranks..."

    output=$(mpirun --allow-run-as-root --oversubscribe -np $np $SOLVER)

    time=$(echo "$output" | grep "Time" | awk '{print $2}')

    if [ -z "$time" ]; then
        time="NaN"
    fi

    echo "$np,$time" >> results.csv
done

echo "Done. Results saved in results.csv"
