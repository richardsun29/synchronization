#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#include "SortedList.h"


int opt_yield = 0;
long long num_iterations = 1;

enum {
	THREADS = 1,
	ITERATIONS,
	YIELD,
	SYNC,
	LISTS
};

void *thread_func(void *arg) {
	return arg;
}

static struct option long_options[] =
{
	{"threads", required_argument, 0, THREADS},
	{"iterations", required_argument, 0, ITERATIONS},
	{"yield", required_argument, 0, YIELD},
	{"sync", required_argument, 0, SYNC},
	{"lists", required_argument, 0, LISTS},
	{0, 0, 0, 0}
};

/* getopt_long stores the option index here. */
int option_index = 0;

int main (int argc, char **argv) {
	long long num_threads = 1;

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
			break;

		case LISTS:
			break;

		default:
			/* code for unrecognized options */
			break;
		}
	}

	/* check for nonpositive values */
	if (num_threads < 1) {
		fprintf(stderr, "Thread number is less than 1\n");
		exit(1);
	}
	if (num_iterations < 1) {
		fprintf(stderr, "Iterations number is less than 1\n");
		exit(1);
	}

	struct timespec start_time;
	if (clock_gettime(CLOCK_MONOTONIC, &start_time)) {
		perror("clock_gettime");
		exit(1);
	}

	pthread_t threads[num_threads];
	int n;
	for (n = 0; n < num_threads; n++) {
		int thread_ret = pthread_create(&threads[n], NULL,
				thread_func, NULL);
		if (thread_ret) {
			perror("pthread_create");
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
		exit(1);
	}

	/*
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
	*/

	SortedList_t *list = SortedList_new_list();
	SortedListElement_t *e1, *e2, *e3, *e4, *e5, *e6;
	e1 = SortedList_new_element("b");
	SortedList_insert(list, e1);
	SortedList_delete(e1);
	e2 = SortedList_new_element("c");
	SortedList_insert(list, e2);
	SortedList_delete(e2);
	e3 = SortedList_new_element("b234");
	SortedList_insert(list, e3);
	SortedList_delete(e3);
	e4 = SortedList_new_element("zzz");
	SortedList_insert(list, e4);
	SortedList_delete(e4);
	e5 = SortedList_new_element("a");
	SortedList_insert(list, e5);
	SortedList_print(list);
	printf("list length with e5: %d\n", SortedList_length(list));
	printf("e5 was found in list: %s\n", SortedList_lookup(list, "a")->key);
	printf("NOTFOUND shouldn't be found, lookup should return NULL: %d\n", SortedList_lookup(list, "NOTHERE") == NULL);
	SortedList_delete(e1);
	SortedList_print(list);
	printf("list length w/o e5: %d\n", SortedList_length(list));

	e6 = SortedList_new_element("Delete this!");
	SortedList_insert(list, e6);
	SortedList_free(list);
	return 0;
}
