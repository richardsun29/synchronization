#!/bin/bash

make -s addtest
mkdir -p graphs data

# get per-op from addtest
# run(nthreads, niterations, sync = none, yield = 0)
run () {
	local threads=$1
	local iterations=$2
	local sync=
	local yield=

	if [ -n "$3" ]; then
		sync="--sync=$3"
	fi
	if [ -n "$4" ]; then
		yield="--yield=1"
	fi

	cmd="./addtest --threads=$threads --iterations=$iterations $sync $yield"
	output="$($cmd 2>/dev/null)"
	echo "$output" | grep "per operation" | awk '{print $3}'
}

# gets average per-op from '$runs' runs
avg_run () {
	local runs=50

	local sum=0
	for i in `seq $runs`; do
		per_op=$(run $@)
		let "sum += $per_op"
	done
	echo $(($sum/$runs))
}


# Threads vs. per operation

threads="$(seq 10)"
iterations=1000

threads_data="data/addtest-threads.dat"
threads_img="graphs/addtest-threads.png"


printf "# Threads\tTime per operation\n" > $threads_data

for nthreads in ${threads[@]}; do
	printf "\r$nthreads threads..."
	per_op=$(avg_run $nthreads $iterations)
	printf "$nthreads\t$per_op\n" >> $threads_data
done

echo "done"

gnuplot -p -e "
set title 'Average Time per Operation vs. Number of Threads ($iterations iterations)';
set key box;
set xlabel 'Number of Threads';
set xrange [0:11];
set ylabel 'Time per Operation (ns)';
set logscale y;
set terminal pngcairo size 800,600 enhanced;
set output \"$threads_img\";
plot \"$threads_data\" using 1:2
title 'no --sync'
"



# Iterations vs. per operation

iterations=(20 100 500 1000 5000 10000)
threads=4

iterations_data="data/addtest-iterations.dat"
iterations_img="graphs/addtest-iterations.png"

printf "# Iterations\tTime per operation\n" > $iterations_data

for niterations in ${iterations[@]}; do
	printf "\r$niterations iterations..."
	per_op=$(avg_run $threads $niterations)
	printf "$niterations\t$per_op\n" >> $iterations_data
done

echo "done"

gnuplot -p -e "
set title 'Average Time per Operation vs. Number of Iterations ($threads threads)';
set xlabel 'Number of Iterations';
unset key;
set xrange [10:100000];
set ylabel 'Time per Operation (ns)';
set logscale;
set offset graph 0.10, 0.10;
set terminal pngcairo size 800,600 enhanced;
set output \"$iterations_img\";
plot \"$iterations_data\" using 1:2
"
