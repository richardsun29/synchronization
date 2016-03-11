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
long long num_lists = 1;
SortedList_t **sorted_lists;
SortedListElement_t **list_elements;
char **keys;
int using_spinlocks = 0, using_mutexes = 0;
void (*insert_func) (SortedList_t*, SortedListElement_t*) = SortedList_insert;
int (*delete_func) (SortedListElement_t*) = SortedList_delete;
SortedListElement_t* (*lookup_func) (SortedList_t*, const char*) = SortedList_lookup;
int (*length_func) (SortedList_t*) = SortedList_length;

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

int list_hash(const char *key) {
	unsigned int hash = 0;
	int i;
	for (i = 0; i < KEY_LENGTH; i++) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash % num_lists;
}

void *thread_func(void *arg) {
	long long thread_num = (long long)arg;
	// insert elements
	int start_elem = thread_num * num_iterations;
	int end_elem = start_elem + num_iterations - 1;
	int i, hash;
	for (i = start_elem; i <= end_elem; i++) {
		hash = list_hash(list_elements[i]->key);
		SortedList_insert(sorted_lists[hash], list_elements[i]);
	}
	//printf("printing list:\n");
	//SortedList_print(sorted_list);

	// count lengths
	int total_length = 0;
	for (i = 0; i < num_lists; i++) {
		int list_size = SortedList_length(sorted_lists[i]);
		if (list_size == -1) {
			fprintf(stderr, "length() detected corrupted list!\n");
			exit(1);
		}
		total_length += list_size;
	}

	// lookup, deletes
	SortedListElement_t *found;
	for (i = start_elem; i <= end_elem; i++) {
		const char *key = list_elements[i]->key;
		hash = list_hash(key);
		found = SortedList_lookup(sorted_lists[hash], key);

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
							exit(1);
					}
				}
			}

			break;
		case SYNC:
			switch(optarg[0]) {
				case 's':
					printf("set spin locks\n");
					using_spinlocks = 1;
					using_mutexes = 0;
					insert_func = SortedList_insert_spinlock;
					delete_func = SortedList_delete_spinlock;
					lookup_func = SortedList_lookup_spinlock;
					length_func = SortedList_length_spinlock;
					break;
				case 'm':
					printf("set mutexes\n");
					using_mutexes = 1;
					using_spinlocks = 0;
					insert_func = SortedList_insert_mutex;
					delete_func = SortedList_delete_mutex;
					lookup_func = SortedList_lookup_mutex;
					length_func = SortedList_length_mutex;
					break;
				default:
					fprintf(stderr, "Unknown option for --sync\n");
					exit(1);
			}
			break;

		case LISTS:
			num_lists = strtol(optarg, NULL, 10);
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
	if (num_lists < 1) {
		fprintf(stderr, "Lists number is less than 1\n");
		exit(1);
	}
	
	sorted_lists = malloc(num_lists * sizeof(SortedList_t*));
	int sl;
	for (sl = 0; sl < num_lists; sl++) {
		sorted_lists[sl] = SortedList_new_list();
	}
	// Initialize spin locks, mutexes
	if (using_spinlocks) {
		spin_locks = malloc(num_lists * sizeof(int));
		for (sl = 0; sl < num_lists; sl++) {
			spin_locks[sl] = 0;
		}
	}
	else if (using_mutexes) {
		blocking_mutexes = (pthread_mutex_t*)malloc(num_lists * sizeof(pthread_mutex_t));
		for (sl = 0; sl < num_lists; sl++) {
			if (pthread_mutex_init(&blocking_mutexes[sl], NULL)) {
				perror("pthread_mutex_init");
				exit(1);
			}
		}
	}
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

	int sorted_list_size = SortedList_length(sorted_lists[0]);
	int length_per_thread = num_iterations / num_lists;
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

	for (sl = 0; sl < num_lists; sl++) {
		SortedList_free(sorted_lists[sl]);
	}
	for (i = 0; i < num_threads * num_iterations; i++) {
		free(keys[i]);
	}
	free(keys);
	free(list_elements);
	if (using_spinlocks) {
		free(spin_locks);
	}
	if (using_mutexes) {
		free(blocking_mutexes);
	}
	return size_err;
	
}
