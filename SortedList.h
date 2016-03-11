#ifndef SORTEDLIST_H
#define SORTEDLIST_H

#include <pthread.h>

/*
 * SortedList (and SortedListElement)
 *
 *	A doubly linked list, kept sorted by a specified key.
 *	This structure is used for a list head, and each element
 *	of the list begins with this structure.
 *
 *	The list head is in the list, and an empty list contains
 *	only a list head.  The list head is also recognizable because
 *	it has a NULL key pointer.
 */
struct SortedListElement {
	struct SortedListElement *prev;
	struct SortedListElement *next;
	const char *key;
};
typedef struct SortedListElement SortedList_t;
typedef struct SortedListElement SortedListElement_t;

/**
 * SortedList_new_list ... create a new sorted list
 *
 *	The list will be initialized with a head node with everything NULL.
 *
 * @return pointer to the SortedList_t, or NULL if creation failed.
 *
 */
SortedList_t *SortedList_new_list();

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
SortedListElement_t *SortedList_new_element(char *key);

/**
 * SortedList_free ... free a sorted list
 *
 *	All memory allocated for the list will be freed
 *
 * @param Sorted_list_t *list ... header for the list to delete
 *
 */
void SortedList_free(SortedList_t *list);

/**
 * SortedList_print ... print all keys in a sorted list
 *
 *	Each key is printed on a separate line
 *
 * @param Sorted_list_t *list ... header for the list to print
 *
 */
void SortedList_print(SortedList_t *list);

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
void SortedList_insert(SortedList_t *list, SortedListElement_t *element);

void SortedList_insert_spinlock(SortedList_t *list, SortedListElement_t *element);

void SortedList_insert_mutex(SortedList_t *list, SortedListElement_t *element);

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
int SortedList_delete(SortedListElement_t *element);

int SortedList_delete_spinlock(SortedListElement_t *element);

int SortedList_delete_mutex(SortedListElement_t *element);

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
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key);

SortedListElement_t *SortedList_lookup_spinlock(SortedList_t *list, const char *key);

SortedListElement_t *SortedList_lookup_mutex(SortedList_t *list, const char *key);

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
int SortedList_length(SortedList_t *list);

int SortedList_length_spinlock(SortedList_t *list);

int SortedList_length_mutex(SortedList_t *list);


/**
 * variable to enable diagnostic calls to pthread_yield
 */
extern int opt_yield;
#define	INSERT_YIELD	0x01	// yield in insert critical section
#define	DELETE_YIELD	0x02	// yield in delete critical section
#define	SEARCH_YIELD	0x04	// yield in lookup/length critical section

int *spin_locks;
pthread_mutex_t *blocking_mutexes;
int list_hash(const char *key);
SortedList_t **sorted_lists;
#endif
