#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <math.h>


static long long counter = 0;

int opt_yield;

pthread_mutex_t mutex;
int spinlock = 0; // 0 = unlocked, 1 = locked

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield)
		pthread_yield();
	*pointer = sum;
}

void *thread_nosync(void *num_iterations) {
	long long i;
	for (i = 0; i < (long long)num_iterations; i++)
		add(&counter, 1);
	for (i = 0; i < (long long)num_iterations; i++)
		add(&counter, -1);
	return 0;
}

void *thread_mutex(void *num_iterations) {
	long long i;
	for (i = 0; i < (long long)num_iterations; i++) {
		pthread_mutex_lock(&mutex);
		add(&counter, 1);
		pthread_mutex_unlock(&mutex);
	}
	for (i = 0; i < (long long)num_iterations; i++) {
		pthread_mutex_lock(&mutex);
		add(&counter, -1);
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}

void *thread_spinlock(void *num_iterations) {
	long long i;
	for (i = 0; i < (long long)num_iterations; i++) {
		while (__sync_lock_test_and_set(&spinlock, 1))
			continue;
		add(&counter, 1);
		__sync_lock_release(&spinlock);
	}
	for (i = 0; i < (long long)num_iterations; i++) {
		while (__sync_lock_test_and_set(&spinlock, 1))
			continue;
		add(&counter, -1);
		__sync_lock_release(&spinlock);
	}
	return 0;
}

void *thread_cas(void *num_iterations) {

}

enum {
	THREADS = 1,
	ITERATIONS,
	YIELD,
	SYNC
};


static struct option long_options[] =
{
	{"threads", required_argument, 0, THREADS},
	{"iterations", required_argument, 0, ITERATIONS},
	{"yield", required_argument, 0, YIELD},
	{"sync", required_argument, 0, SYNC},
	{0, 0, 0, 0}
};

/* getopt_long stores the option index here. */
int option_index = 0;

int main (int argc, char **argv) {
	long long num_threads = 1;
	long long num_iterations = 1;

	opt_yield = 0;
	void *(*thread_func)(void *) = &thread_nosync;


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

		case YIELD:
			opt_yield = strtol(optarg, NULL, 10);
			break;

		case SYNC:
			switch(optarg[0]) {
			case 'm':
				thread_func = thread_mutex;
				if (pthread_mutex_init(&mutex, NULL)) {
					perror("pthread_mutex_init");
					exit(1);
				}
				break;
			case 's':
				thread_func = thread_spinlock;
				break;
			case 'c':
				thread_func = thread_cas;
				break;
			default:
				fprintf(stderr,
				"Unknown option for --sync\n");
			}
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

	struct timespec start_time;
	if (clock_gettime(CLOCK_MONOTONIC, &start_time)) {
		perror("clock_gettime");
	}

	pthread_t threads[num_threads];
	int n;
	for (n = 0; n < num_threads; n++) {
		int thread_ret = pthread_create(&threads[n], NULL,
				thread_func, (void*) num_iterations);
		if (thread_ret) {
			perror("pthread_join");
			exit(1);
		}
	}
	for (n = 0; n < num_threads; n++) {
		int join_ret = pthread_join(threads[n], NULL);
		if (join_ret) {
			perror("pthread_join");
			exit(1);
		}
	}

	struct timespec end_time;
	if (clock_gettime(CLOCK_MONOTONIC, &end_time)) {
		perror("clock_gettime");
	}

	long long operations = num_threads * num_iterations * 2;
	printf("%lld threads x %lld iterations x (add + subtract) = %lld operations\n",
		num_threads, num_iterations, operations);

	int counter_err = counter != 0;
	if (counter_err) {
		fprintf(stderr, "ERROR: final count = %lld\n", counter);
	}

	long long elapsed = (end_time.tv_sec * pow(10, 9) + end_time.tv_nsec)
			- (start_time.tv_sec * pow(10, 9) + start_time.tv_nsec);
	printf("elapsed time: %lld ns\n", elapsed);
	printf("per operation: %lld ns\n", elapsed / operations);

	return counter_err;
}
