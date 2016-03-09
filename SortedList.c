#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
	/*
	SortedList_t *list = (SortedList_t*)malloc(sizeof(SortedList_t));
	if (list == NULL)
		return NULL;
	list->prev = NULL;
	list->next = NULL;
	list->key  = NULL;
	return list;
	*/
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
	element->key  = key;
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
void SortedList_free(SortedList_t *list);

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
		printf("%s\n", curr->key);
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
	element->prev = prev;
	if (next != NULL) { // not inserting at end of list
		next->prev = element;
	}
	prev->next = element;
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
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 * Note: if (opt_yield & DELETE_YIELD)
 *		call pthread_yield in middle of critical section
 */
int SortedList_delete( SortedListElement_t *element);

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

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
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
