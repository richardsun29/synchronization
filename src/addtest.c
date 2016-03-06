#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>


static long long counter = 0;

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	*pointer = sum;
}

void *thread_func(void *num_iterations) {
	long long i;
	for (i = 0; i < (long long)num_iterations; i++)
		add(&counter, 1);
	for (i = 0; i < (long long)num_iterations; i++)
		add(&counter, -1);
	return 0;
}

enum {
	THREADS = 1,
	ITERATIONS,
	LISTS,
	SYNC
};


static struct option long_options[] =
{
	{"threads", required_argument, 0, THREADS},
	{"iterations", required_argument, 0, ITERATIONS},
	{"lists", required_argument, 0, LISTS},
	{"sync", required_argument, 0, SYNC},
	{0, 0, 0, 0}
};

/* getopt_long stores the option index here. */
int option_index = 0;

int main (int argc, char **argv) {
	long long num_threads = 1;
	long long num_iterations = 1;

	int c;
	while (1)
	{
		c = getopt_long(argc, argv, "", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case THREADS:
				num_threads = strtol(optarg, NULL, 10);
				break;

			case ITERATIONS:
				num_iterations = strtol(optarg, NULL, 10);
				break;

			case LISTS:
				break;

			case SYNC:
				break;

			default:
				/* code for unrecognized options */
				break;
		}
	}

	/* check for nonpositive values */
	if (num_threads < 1)
		num_threads = 1;
	if (num_iterations < 1)
		num_iterations = 1;

	pthread_t threads[num_threads];
	int n;
	for (n = 0; n < num_threads; n++) {
		int thread_ret = pthread_create(&threads[n], NULL, thread_func, (void*) num_iterations);
		if (thread_ret) {
			fprintf(stderr, "Thread couldn't be created!\n");
			exit(1);
		}
	}
	for (n = 0; n < num_threads; n++) {
		int join_ret = pthread_join(threads[n], NULL);
		if (join_ret) {
			fprintf(stderr, "Thread couldn't be joined!\n");
			exit(1);
		}
	}

	int counter_err = counter != 0;
	if (counter_err) {
		fprintf(stderr, "ERROR: final count = %lld\n", counter);
	}

	return counter_err;
}
