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
SortedList_t *sorted_list;
SortedListElement_t **list_elements;
char **keys;

enum {
	THREADS = 1,
	ITERATIONS,
	YIELD,
	SYNC,
	LISTS
};

#define KEY_LENGTH 10
char *rand_key(void) {
	char *key = (char*)malloc(sizeof(char) * (KEY_LENGTH + 1));
	int i;
	for (i = 0; i < KEY_LENGTH; i++) {
		key[i] = 'A' + (rand() % 26);
	}
	key[KEY_LENGTH] = '\0';
	return key;
}

void *thread_func(void *arg) {
	long long thread_num = (long long)arg;
	// insert elements
	int start_elem = thread_num * num_iterations;
	int end_elem = start_elem + num_iterations - 1;
 	int i;
	for (i = start_elem; i <= end_elem; i++) {
		SortedList_insert(sorted_list, list_elements[i]);
	}
	//printf("printing list:\n");
	//SortedList_print(sorted_list);
	// count length
	int list_size = SortedList_length(sorted_list);
	if (list_size == -1) {
		fprintf(stderr, "length() detected corrupted list!\n");
		exit(1);
	}

	// lookup, deletes
	SortedListElement_t *found;
	for (i = start_elem; i <= end_elem; i++) {
		found = SortedList_lookup(sorted_list, list_elements[i]->key);
		if (found == NULL) {
			fprintf(stderr, "lookup() did not find key!\n");
			exit(1);
		}
		if (SortedList_delete(found)) {
			fprintf(stderr, "delete() detected corrupted list!\n");
			exit(1);
		}
	}
	return (void*)arg;
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
	srand(time(0));
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
			{
				int i;
				for (i = 0; optarg[i] != 0; i++) {
					switch(optarg[i]) {
						case 'i':
							opt_yield |= INSERT_YIELD;	
							break;
						case 'd':
							opt_yield |= DELETE_YIELD;
							break;
						case 's':
							opt_yield |= SEARCH_YIELD;
							break;
						default:
							fprintf(stderr, "Unknown option for --yield\n");
					}
				}
			}

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

	sorted_list = SortedList_new_list();
	pthread_t threads[num_threads];
	list_elements = malloc(num_threads * num_iterations * sizeof(SortedListElement_t*));
	keys = (char**)malloc(num_threads * num_iterations * sizeof(char*));

	// Create elements with random keys
	int i;
	for (i = 0; i < num_threads * num_iterations; i++) {
		keys[i] = rand_key();
		list_elements[i] = SortedList_new_element(keys[i]);
	}

	struct timespec start_time;
	if (clock_gettime(CLOCK_MONOTONIC, &start_time)) {
		perror("clock_gettime");
		exit(1);
	}

	long long n;
	for (n = 0; n < num_threads; n++) {
		int thread_ret = pthread_create(&threads[n], NULL,
				thread_func, (void*)n);
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

	int sorted_list_size = SortedList_length(sorted_list);
	int length_per_thread = num_iterations / num_threads;
	long long operations = num_threads * num_iterations * length_per_thread;
	printf("%lld threads x %lld iterations x (ins + lookup/del) x (%d/2 avg len) = %lld operations\n",
		num_threads, num_iterations, length_per_thread, operations);

	int size_err = sorted_list_size != 0;
	if (size_err) {
		fprintf(stderr, "ERROR: final count = %d\n", sorted_list_size);
	}

	long long elapsed = (end_time.tv_sec * pow(10, 9) + end_time.tv_nsec)
			- (start_time.tv_sec * pow(10, 9) + start_time.tv_nsec);
	printf("elapsed time: %lld ns\n", elapsed);
	printf("per operation: %lld ns\n", elapsed / operations);

	SortedList_free(sorted_list);
	for (i = 0; i < num_threads * num_iterations; i++) {
		free(keys[i]);
	}
	free(keys);
	free(list_elements);

	return size_err;
	

	/*SortedList_t *list = SortedList_new_list();
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
	SortedList_free(list);*/


	//return 0;
}
