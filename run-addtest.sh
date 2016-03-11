#!/bin/bash

make -s addtest
mkdir -p graphs data

# get per-op from addtest
# run(nthreads, niterations, sync = none, yield = 0)
run () {
	local threads="--threads=$1"
	local iterations="--iterations=$2"
	local sync=
	local yield=

	if [ -n "$3" ]; then
		sync="--sync=$3"
	fi
	if [ -n "$4" ]; then
		yield="--yield=1"
	fi

	cmd="./addtest $threads $iterations $sync $yield"
	output="$($cmd 2>/dev/null)"
	echo "$output" | grep "per operation" | awk '{print $3}'
}

# gets average per-op from '$runs' runs
avg_run () {
	local runs=2

	local sum=0
	for i in `seq $runs`; do
		per_op=$(run $@)
		let "sum += $per_op"
	done
	echo $(($sum/$runs))
}


# per operation vs. threads

max_threads=20
iterations=10000

threads_data="data/addtest-threads.dat"
threads_img="graphs/addtest-threads.png"

printf "#threads\tnosync\tmutex\tspin\tcas\n" > $threads_data

nthreads=1
while [ $nthreads -le $max_threads ]; do
	printf "\r$nthreads threads..."
	avg_n=$(avg_run $nthreads $iterations)
	avg_m=$(avg_run $nthreads $iterations m)
	avg_s=$(avg_run $nthreads $iterations s)
	avg_c=$(avg_run $nthreads $iterations c)
	printf "$nthreads\t$avg_n\t$avg_m\t$avg_s\t$avg_c\n" >> $threads_data
	let "nthreads++"
done

echo "done"

gnuplot -e "
set title 'addtest: Avg. Time per Operation vs. Number of Threads ($iterations iterations)';
set key box;
set key left top;
set xlabel 'Number of Threads';
set xrange [0:$(($max_threads+1))];
set ylabel 'Time per Operation (ns)';
set terminal pngcairo size 800,600 enhanced;
set output \"$threads_img\";
plot \"$threads_data\" using 1:2 title 'no sync',  \
     \"$threads_data\" using 1:3 title '--sync=m', \
     \"$threads_data\" using 1:4 title '--sync=s', \
     \"$threads_data\" using 1:5 title '--sync=c'
"



# per operation vs. iterations

max_iterations=10000000
threads=1

iterations_data="data/addtest-iterations.dat"
iterations_img="graphs/addtest-iterations.png"

printf "# Iterations\tTime per operation\n" > $iterations_data

niterations=1
while [ $niterations -le $max_iterations ]; do
	printf "\r$niterations iterations..."
	per_op=$(avg_run $threads $niterations)
	printf "$niterations\t$per_op\n" >> $iterations_data
	let "niterations *= 2"
done

echo "done"

gnuplot -e "
set title 'addtest: Avg. Time per Operation vs. Number of Iterations';
set xlabel 'Number of Iterations';
unset key;
set xrange [1:$max_iterations];
set ylabel 'Time per Operation (ns)';
set logscale;
set offset graph 0.10, 0.10;
set terminal pngcairo size 800,600 enhanced;
set output \"$iterations_img\";
plot \"$iterations_data\" using 1:2
"
