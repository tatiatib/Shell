#ifndef HW1_SHELL_TIMMY_DICTIONARY_H
#define HW1_SHELL_TIMMY_DICTIONARY_H

#define ALLOWED_CHARACTER_COUNT 63

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "helper.h"

/**
 * Type: DictionaryFreeFunction
 * ---------------------------------
 * DictionaryFreeFunction defines the space of functions that can be used as the
 * clean-up function for each element as it is deleted from the dictionary
 * or when the entire dictionary is destroyed.  The cleanup function is
 * called with a pointer to an element about to be deleted.
 */
typedef void (*DictionaryFreeFunction)(void *elemAddr);

/**
 * Type: dictionary
 * -------------
 * The concrete representation of the dictionary, 
 * where the keys are strings associated with elements
 * and the values are pointers to said elements.
 *
 * !!! ALLOWED CHARACTERS !!!
 * English letters ([A-Z], [a-z]), digits (0-9), underscore sign "_".
 *
 * In spite of all of the fields being publicly accessible, the
 * client is absolutely required to initialize, dispose of, and
 * otherwise interact with all dictionary instances via the suite
 * of the dictionary-related functions described below.
 */
typedef struct {
    int n_children;
    size_t count;
    void *value;
    struct DictionaryNode *children[ALLOWED_CHARACTER_COUNT];
    DictionaryFreeFunction freeFn;
} DictionaryNode;

/*
 * Function:  dictionary_new
 * ---------------------
 * Initializes the identified dictionary to be empty.  It is assumed that
 * the specified dictionary is either raw memory or one previously initialized
 * but later destroyed (using dictionary_dispose.)
 * 
 * The freefn is the function that will be called on an element that is
 * about to be overwritten (by a new entry in dictionary) or on each element
 * in the table when the entire table is being freed (using dictionary_dispose).  This 
 * function is your chance to do any deallocation/cleanup required,
 * (such as freeing any pointers contained in the element). The client can pass 
 * NULL for the freefn if the elements don't require any handling. 
 *
 */
void dictionary_new(DictionaryNode *node, DictionaryFreeFunction freeFn);

/*
 * Function: dictionary_dispose
 * ------------------------
 * Disposes of any resources acquired during the lifetime of the
 * dictionary.  It does not dispose of client elements properly unless the
 * DictionaryFreeFunction specified at construction time does the right
 * thing.  dictionary_dispose will apply this cleanup routine to all
 * of the client elements stored within.
 *
 * Once dictionary_dispose has been called, the dictionary is rendered
 * useless.  The diposed of dictionary should not be passed to any
 * other dictionary routines (unless for some reason you want to reuse
 * the same dictionary variable and re-initialize is by passing it to
 * dictionary_new... not recommended.)
 */
void dictionary_dispose(DictionaryNode *d);

/*
 * Function: dictionary_size
 * ----------------------
 * Returns the number of elements residing in
 * the specified dictionary.
 */
size_t dictionary_size(const DictionaryNode *d);


/*
 * Function: dictionary_put
 * ----------------------
 * Assigns the specified element to the given key into the specified
 * dictionary.  If the specified key matches a
 * key previously inserted, then the value of the
 * old key is replaced by this new element.
 *
 * An assert is raised if the specified key or value address is NULL,
 * or if the given key contains an unallowed character.
 */
void dictionary_put(DictionaryNode *d, const char *key, const void *value);

/*
 * Function: dictionary_get
 * -----------------------
 * Examines the specified dictionary to see if anything matches
 * the specified key.  If a match is found,
 * then the address of the stored item is returned.
 * If no match is found, then NULL is returned as a sentinel.
 *
 * An assert is raised if the specified key is NULL
 * or contains an unallowed character.
 */

void *dictionary_get(const DictionaryNode *d, const char *key);

/*
 * Function: dictionary_remove
 * -----------------------
 * Examines the specified dictionary to see if anything matches
 * the specified key. If a match is found,
 * the element is no longer associated with the specified key
 * and its value is returned. Additionally,
 * a cleanup routine will be applied to the element, if present.
 * If no match is found, then NULL is returned as a sentinel.
 *
 * An assert is raised if the specified key is NULL
 * or contains an unallowed character.
 *
 */
void *dictionary_remove(DictionaryNode *d, const char *key);

#endif