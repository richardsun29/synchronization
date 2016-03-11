#!/bin/bash

make -s sltest
mkdir -p graphs data

# get per-op from sltest
# run(nthreads, niterations, sync = none, nlists = 1, yield = 0)
run () {
	local threads="--threads=$1"
	local iterations="--iterations=$2"
	local sync=
	local lists=
	local yield=

	if [ -n "$3" ]; then
		sync="--sync=$3"
	fi
	if [ -n "$4" ]; then
		lists="--lists=$4"
	fi
	if [ -n "$5" ]; then
		yield="--yield=$5"
	fi

	cmd="./sltest $threads $iterations $lists $sync $yield"
	output="$($cmd 2>/dev/null)"
	echo "$output" | grep "per operation" | awk '{print $3}'
}

# gets average per-op from '$runs' runs
avg_run () {
	local runs=3

	local sum=0
	for i in `seq $runs`; do
		per_op=$(run $@)
		let "sum += $per_op"
	done
	echo $(($sum/$runs))
}


# per operation vs. iterations

max_iterations=100000
threads=1

iterations_data="data/sltest-iterations.dat"
iterations_img="graphs/sltest-iterations.png"

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
set title 'sltest: Avg. Time per Operation vs. Number of Iterations';
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


# per operation vs. threads
exit

max_threads=10
iterations=1000

threads_data="data/sltest-threads.dat"
threads_img="graphs/sltest-threads.png"

printf "#threads\tnosync\tmutex\tspin\n" > $threads_data

nthreads=1
while [ $nthreads -le $max_threads ]; do
	printf "\r$nthreads threads..."
	avg_n=$(avg_run $nthreads $iterations)
	avg_m=$(avg_run $nthreads $iterations m)
	avg_s=$(avg_run $nthreads $iterations s)
	printf "$nthreads\t$avg_n\t$avg_m\t$avg_s\n" >> $threads_data
	let "nthreads++"
done

echo "done"

gnuplot -e "
set title 'sltest: Avg. Time per Operation vs. Number of Threads ($iterations iterations)';
set key box;
set key left top;
set xlabel 'Number of Threads';
set xrange [0:$(($max_threads+1))];
set ylabel 'Time per Operation (ns)';
set terminal pngcairo size 800,600 enhanced;
set output \"$threads_img\";
plot \"$threads_data\" using 1:2 title 'no sync',  \
     \"$threads_data\" using 1:3 title '--sync=m', \
     \"$threads_data\" using 1:4 title '--sync=s'
"

# per operation vs. thread:list ratio

max_threads=10
max_lists=10
iterations=1000

threads_data="data/sltest-lists.dat"
threads_img="graphs/sltest-lists.png"

printf "#ratio\tnosync\tmutex\tspin\n" > $threads_data

nthreads=1
while [ $nthreads -le $max_threads ]; do
	nlists=1
	while [ $nlists -le $max_lists ]; do
		printf "\r$nthreads threads..."
		avg_n=$(avg_run $nthreads $iterations '' $nlists)
		avg_m=$(avg_run $nthreads $iterations m  $nlists)
		avg_s=$(avg_run $nthreads $iterations s  $nlists)
		ratio=bc <<< "scale = 4; $nthreads / $nlists" # 4 decimals
		printf "$ratio\t$avg_n\t$avg_m\t$avg_s\n" >> $threads_data
		let "nlists++"
	done
	let "nthreads++"
done

echo "done"

gnuplot -e "
set title 'sltest: Avg. Time per Operation vs. Threads:Lists ratio ($iterations iterations)';
set key box;
set key left top;
set xlabel 'Number of Threads';
set xrange [0:$(($max_threads+1))];
set ylabel 'Time per Operation (ns)';
set terminal pngcairo size 800,600 enhanced;
set output \"$threads_img\";
plot \"$threads_data\" using 1:2 title 'no sync',  \
     \"$threads_data\" using 1:3 title '--sync=m', \
     \"$threads_data\" using 1:4 title '--sync=s'
"
