#!/bin/bash

threads=(1 2 3 4 6 8 10)
iterations=(20 100 500 1000 5000 10000)

data_file="addtest.dat"
threads_img="addtest-threads.png"
iterations_img="addtest-iterations.png"

make -s clean addtest

printf "# Threads\tIterations\tTime per operation\n" > $data_file

for nthreads in ${threads[@]}; do
	for niterations in ${iterations[@]}; do
		cmd="./addtest --threads=$nthreads --iterations=$niterations"
		#echo $cmd
		output="$($cmd 2>/dev/null)"
		per_op=$(echo "$output" | grep "per operation" | awk '{print $3}')
		printf "$nthreads\t$niterations\t$per_op\n" >> $data_file
	done
done

gnuplot -p -e "
set xlabel 'Number of Threads';
set xrange [0:11];
set ylabel 'Time per Operation (ns)';
set logscale y;
set terminal pngcairo size 800,600 enhanced;
set output \"$threads_img\";
plot \"$data_file\" using 1:3 title 'Time per Operation vs. Number of Threads'
"

gnuplot -p -e "
set xlabel 'Number of Iterations';
set xrange [10:100000];
set ylabel 'Time per Operation (ns)';
set logscale;
set offset graph 0.10, 0.10;
set terminal pngcairo size 800,600 enhanced;
set output \"$iterations_img\";
plot \"$data_file\" using 2:3 title 'Time per Operation vs. Number of Iterations'
"
