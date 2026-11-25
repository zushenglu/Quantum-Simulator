#!/bin/bash
#SBATCH --nodes=1
#SBATCH --cpus-per-task=40
#SBATCH --time=1:00:00
#SBATCH --job-name=simulate_strong
#SBATCH --output=simulate_strong-%j.out

set -euo pipefail
TIMEFORMAT='%3R'

pwd
# Run from the repository root.
circuit="./test_circuit"
size="10" # 8 through 16
size="${1-$size}"

run_exp() {
    local circuit=$1 thread=$2 algo=$3 elapsed
    # echo "Testing the $circuit circuit with $thread thread(s)"
    export OMP_NUM_THREADS=$thread
    elapsed=$(
        { time "$circuit" "$algo" "$size" >/dev/null 2>&3; } 3>&2 2>&1
    )
    printf '%s,%s,%s,%s\n' "$size" "$algo" "$thread" "$elapsed" >> "$output"
}

mkdir -p benchmark
run_dir=$(mktemp -d "benchmark/simulate_strong_$(date +%Y%m%d_%H%M%S)_XXXXXX")
output="$run_dir/result.csv"
printf 'qubit,algo,thread,time\n' > "$output"
# for algo in 0 2; do
for thread in 1 2 3 4 8 16 24 32 40; do
    if (( thread == 1 )); then
        run_exp "$circuit" "$thread" "0"
    else
        run_exp "$circuit" "$thread" "2"
    fi
    
done
# done

echo "Saved timings to $output"
