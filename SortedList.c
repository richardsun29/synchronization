#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <pthread.h>

#include "SortedList.h"


/**
 * SortedList_new_list ... create a new sorted list
 *
 *	The list will be initialized with a head node with everything NULL.
 *
 * @return pointer to the SortedList_t, or NULL if creation failed.
 *
 */
SortedList_t *SortedList_new_list() {
	// a SortedList_t is just a SortedListElement_t with a NULL key
	return (SortedList_t*)SortedList_new_element(NULL);
}

/**
 * SortedList_new_element ... create a new sorted list element
 *
 *	The element will be initialized with the given key, and .prev and .next
 *	are NULL.
 *
 * @param char *key ... key for this element
 * @return pointer to the SortedList_t, or NULL if creation failed.
 *
 */
SortedListElement_t *SortedList_new_element(char *key) {
	SortedListElement_t *element = 
		(SortedListElement_t*)malloc(sizeof(SortedListElement_t));
	if (element == NULL)
		return NULL;
	element->prev = NULL;
	element->next = NULL;
	element->key = key;
	return element;
}

/**
 * SortedList_free ... free a sorted list
 *
 *	All memory allocated for the list will be freed
 *
 * @param Sorted_list_t *list ... header for the list to delete
 *
 */
void SortedList_free(SortedList_t *list) {
	SortedListElement_t *curr = list->next;
	while (curr != NULL) {
		SortedListElement_t *del = curr;
		curr = curr->next;
		free(del);
	}
	free(list);
}

/**
 * SortedList_print ... print all keys in a sorted list
 *
 *	Each key is printed on a separate line
 *
 * @param Sorted_list_t *list ... header for the list to print
 *
 */
void SortedList_print(SortedList_t *list) {
	SortedListElement_t *curr = list->next;
	while (curr != NULL) {
		// printf("%s\n", curr->key);
		curr = curr->next;
	}
}

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 *
 * Note: if (opt_yield & INSERT_YIELD)
 *		call pthread_yield in middle of critical section
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {

	// find nodes to insert element between
	SortedListElement_t *prev = list,
			    *next = list->next;

	while (next != NULL && strcmp(element->key, next->key) > 0) {
		prev = next;
		next = next->next;
	}

	element->next = next;

	if (opt_yield & INSERT_YIELD) {
		// printf("Insert yielding\n");
		pthread_yield();
	}

	element->prev = prev;
	if (next != NULL) { // not inserting at end of list
		next->prev = element;
	}
	prev->next = element;
}

void SortedList_insert_spinlock(SortedList_t *list, SortedListElement_t *element) {
	int list_index = list_hash(element->key);
	while (__sync_lock_test_and_set(&spin_locks[list_index], 1))
		continue;
	// find nodes to insert element between
	SortedListElement_t *prev = list,
			    *next = list->next;

	while (next != NULL && strcmp(element->key, next->key) > 0) {
		prev = next;
		next = next->next;
	}

	element->next = next;

	if (opt_yield & INSERT_YIELD) {
		// printf("Insert yielding\n");
		pthread_yield();
	}

	element->prev = prev;
	if (next != NULL) { // not inserting at end of list
		next->prev = element;
	}
	prev->next = element;

	__sync_lock_release(&spin_locks[list_index]);
}
void SortedList_insert_mutex(SortedList_t *list, SortedListElement_t *element) {
	int list_index = list_hash(element->key);
	pthread_mutex_lock(&blocking_mutexes[list_index]);

	// find nodes to insert element between
	SortedListElement_t *prev = list,
			    *next = list->next;
	while (next != NULL && strcmp(element->key, next->key) > 0) {
		prev = next;
		next = next->next;
	}

	element->next = next;

	if (opt_yield & INSERT_YIELD) {
		// printf("Insert yielding\n");
		pthread_yield();
	}

	element->prev = prev;
	if (next != NULL) { // not inserting at end of list
		next->prev = element;
	}
	prev->next = element;

	pthread_mutex_unlock(&blocking_mutexes[list_index]);

}
/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrupted prev/next pointers
 *
 * Note: if (opt_yield & DELETE_YIELD)
 *		call pthread_yield in middle of critical section
 */
int SortedList_delete(SortedListElement_t *element) {
	// check for corrupted prev/next pointers
	// element->prev should never be NULL
	if ((element->next != NULL && element->next->prev != element)
	    || element->prev->next != element) {
		return 1;
	}

	element->prev->next = element->next;

	if (opt_yield & DELETE_YIELD) {
		// printf("Delete yielding\n");
		pthread_yield();
	}

	if (element->next != NULL) {
		element->next->prev = element->prev;
	}

	free(element);
	return 0;
}
int SortedList_delete_spinlock(SortedListElement_t *element) {
	int list_index = list_hash(element->key);
	while (__sync_lock_test_and_set(&spin_locks[list_index], 1))
		continue;

	// check for corrupted prev/next pointers
	// element->prev should never be NULL
	if ((element->next != NULL && element->next->prev != element)
	    || element->prev->next != element) {
		return 1;
	}

	element->prev->next = element->next;

	if (opt_yield & DELETE_YIELD) {
		// printf("Delete yielding\n");
		pthread_yield();
	}

	if (element->next != NULL) {
		element->next->prev = element->prev;
	}

	__sync_lock_release(&spin_locks[list_index]);
	free(element);
	return 0;
}
int SortedList_delete_mutex(SortedListElement_t *element) {
	int list_index = list_hash(element->key);
	pthread_mutex_lock(&blocking_mutexes[list_index]);

	// check for corrupted prev/next pointers
	// element->prev should never be NULL
	if ((element->next != NULL && element->next->prev != element)
	    || element->prev->next != element) {
		return 1;
	}

	element->prev->next = element->next;

	if (opt_yield & DELETE_YIELD) {
		// printf("Delete yielding\n");
		pthread_yield();
	}

	if (element->next != NULL) {
		element->next->prev = element->prev;
	}
	
	pthread_mutex_unlock(&blocking_mutexes[list_index]);

	free(element);
	return 0;
}
/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 *
 * Note: if (opt_yield & SEARCH_YIELD)
 *		call pthread_yield in middle of critical section
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	SortedListElement_t *element = (SortedListElement_t*)list;
	while (element->next != NULL) {
		element = element->next;

		if (opt_yield & SEARCH_YIELD) {
			// printf("Lookup yielding\n");
			pthread_yield();
		}

		if (strcmp(element->key, key) == 0) {
			return element;
		}
	}
	return NULL;
}
SortedListElement_t *SortedList_lookup_spinlock(SortedList_t *list, const char *key) {
	int list_index = list_hash(key);
	// printf("spinlock lookup\n");
	while (__sync_lock_test_and_set(&spin_locks[list_index], 1))
		continue;
	SortedListElement_t *element = (SortedListElement_t*)list;
	while (element->next != NULL) {
		element = element->next;

		if (opt_yield & SEARCH_YIELD) {
			// printf("Lookup yielding\n");
			pthread_yield();
		}

		if (strcmp(element->key, key) == 0) {
			__sync_lock_release(&spin_locks[list_index]);
			return element;
		}
	}
	__sync_lock_release(&spin_locks[list_index]);
	return NULL;
}
SortedListElement_t *SortedList_lookup_mutex(SortedList_t *list, const char *key) {
	SortedListElement_t *element = (SortedListElement_t*)list;
	int list_index = list_hash(key);
	// printf("mutex lookup\n");
	pthread_mutex_lock(&blocking_mutexes[list_index]);

	while (element->next != NULL) {
		element = element->next;

		if (opt_yield & SEARCH_YIELD) {
			// printf("Lookup yielding\n");
			pthread_yield();
		}

		if (strcmp(element->key, key) == 0) {
			pthread_mutex_unlock(&blocking_mutexes[list_index]);

			return element;
		}
	}
	pthread_mutex_unlock(&blocking_mutexes[list_index]);

	return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumerating list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 *
 * Note: if (opt_yield & SEARCH_YIELD)
 *		call pthread_yield in middle of critical section
 */
int SortedList_length(SortedList_t *list) {
	SortedListElement_t *element = (SortedListElement_t*)list->next;
	if (element == NULL) {
		return 0;
	}
	int length = 1;
	while (element->next != NULL) {
		if ((element->next != NULL && element->next->prev != element)
				|| element->prev->next != element) {
			return -1;
		}
		element = element->next;
		if (opt_yield & SEARCH_YIELD) {
			// printf("Length yielding\n");
			pthread_yield();
		}
		length++;
	}
	return length;
}
int SortedList_length_spinlock(SortedList_t *list) {
	int list_index = 0;
	while (sorted_lists[list_index] != list)
		list_index++;
	int length = 1;

	while (__sync_lock_test_and_set(&spin_locks[list_index], 1))
		continue;

	SortedListElement_t *element = (SortedListElement_t*)list->next;
	if (element == NULL) {
		__sync_lock_release(&spin_locks[list_index]);
		return 0;
	}
	while (element->next != NULL) {
		if ((element->next != NULL && element->next->prev != element)
				|| element->prev->next != element) {
			__sync_lock_release(&spin_locks[list_index]);
			return -1;
		}
		element = element->next;
		if (opt_yield & SEARCH_YIELD) {
			// printf("Length yielding\n");
			pthread_yield();
		}
		length++;
	}
	__sync_lock_release(&spin_locks[list_index]);
	return length;
}
int SortedList_length_mutex(SortedList_t *list) {
	int list_index = 0;
	while (sorted_lists[list_index] != list)
		list_index++;
	int length = 1;

	pthread_mutex_lock(&blocking_mutexes[list_index]);

	SortedListElement_t *element = (SortedListElement_t*)list->next;
	if (element == NULL) {
		pthread_mutex_unlock(&blocking_mutexes[list_index]);
		return 0;
	}

	while (element->next != NULL) {
		if ((element->next != NULL && element->next->prev != element)
				|| element->prev->next != element) {
			pthread_mutex_unlock(&blocking_mutexes[list_index]);

			return -1;
		}
		element = element->next;
		if (opt_yield & SEARCH_YIELD) {
			// printf("Length yielding\n");
			pthread_yield();
		}
		length++;
	}
	pthread_mutex_unlock(&blocking_mutexes[list_index]);

	return length;
}

