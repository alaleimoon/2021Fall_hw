#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "gmap.h"
#include "entry.h"



typedef struct _node
{
    void* key;
    void* value;
    struct _node* next;
}node;

struct _gmap
{
    node **table;
    size_t capacity;
    size_t size;
    //size_t num_chains;
    
    size_t (*hash)(const void *);
    int (*compare)(const void *, const void *);
    void *(*copy)(const void *);
    void (*free)(void *);
};

void gmap_embiggen(gmap *m);

size_t gmap_table_find_key(const node *table, const void *key,
    size_t (*hash)(const void *),
    int (*compare)(const void *, const void *), size_t size, size_t capacity);

#define GMAP_INITIAL_CAPACITY 64




/**
 * Creates an empty map that uses the given hash function.
 *
 * @param cp a function that take a pointer to a key and returns a pointer to a deep copy of that key
 * @param comp a pointer to a function that takes two keys and returns the result of comparing them,
 * with return value as for strcmp
 * @param h a pointer to a function that takes a pointer to a key and returns its hash code
 * @param f a pointer to a function that takes a pointer to a copy of a key make by cp and frees it
 * @return a pointer to the new map or NULL if it could not be created;
 * it is the caller's responsibility to destroy the map
 */
gmap *gmap_create(void *(*cp)(const void *), int (*comp)(const void *, const void *), size_t (*h)(const void *s), void (*f)(void *)){
    if (cp == NULL || comp == NULL || h == NULL || f == NULL){
        //missing function
        return NULL;
    }

    gmap *result = malloc(sizeof(gmap));
    if (result != NULL){
        //remember the functions used to manipulate the keys
        result->copy = cp;
        result->compare = comp;
        result->hash = h;
        result->free = f;

        //initialize the table
        result->table = malloc(sizeof(node *) * GMAP_INITIAL_CAPACITY);
        result->capacity = (result->table != NULL ? GMAP_INITIAL_CAPACITY : 0);
        result->size = 0;
    }
    return result;
}


/**
 * Returns the number of (key, value) pairs in the given map.
 *
 * @param m a pointer to a map, non-NULL
 * @return the size of the map pointed to by m
 */
size_t gmap_size(const gmap *m){
    if(m == NULL){
        return 0;
    }
    return m->size;
}


/**
 * Adds a copy of the given key with value to this map.  If the key is
 * already present then the old value is replaced and returned.  The
 * map copies the key, so the caller retains ownership of the original
 * key and may modify it or destroy it without affecting the map.  The
 * map copies the pointer to the value, but the caller retains
 * ownership of the value.
 *
 * @param m a pointer to a map, non-NULL
 * @param key a pointer to a key, non-NULL
 * @param value a pointer to a value
 * @return a pointer to the old value, or NULL, or a pointer to gmap_error
 */
void *gmap_put(gmap *m, const void *key, void *value){
    if(m == NULL || key == NULL){
        return NULL;
    }
    if(m->size == m->capacity){
        //table is full
        gmap_embiggen(m);
    }

    //find key
    TODO: change here!!!
    int index = gmap_table_find_key(m->table, key, m->hash, m->compare, m->size, m->capacity);
    if(index < m->size){
        //key already present, replace old value and return the old one
        void *old_value = m->table[index].key;
        m->table[index].value = value;
        return old_value;
    }
    else{
        //new key, check whether there is room for the new entry
        if(m->size == m->capacity){
            //change!!!!
            char *gmap_error = "no_room_for_key";
            return gmap_error;
        }
        void *copy = m->copy(key);
        if(copy != NULL){
            m->table[index].key = copy;
            m->table[index].value = value;
            m->size++;
            return NULL;
        }
        else{
            return NULL;
        }
    }
    return NULL;
}

void gmap_embiggen(gmap *m)
{
  // TO-DO: write and test embiggen
  return;
}

node *gmap_table_find_key(const node **table, const void *key,
        size_t (*hash)(const void *),
        int (*compare)(const void *, const void *), size_t size, size_t capacity){
    size_t i = hash(key) % capacity;
    node *curr = table[i];
    while(curr != NULL && compare(table[i].key, key) != 0){
        curr = curr->next;
    }
    return curr;
}

/**
 * Removes the given key and its associated value from the given map if
 * the key is present.  The return value is NULL and there is no effect
 * on the map if the key is not present.  They copy of the key held by
 * the map is destroyed.  It is the caller's responsibility to free the
 * returned value if necessary.
 *
 * @param m a pointer to a map, non-NULL
 * @param key a key, non-NULL
 * @return the value associated with the removed key, or NULL
 */
void *gmap_remove(gmap *m, const void *key){
    return NULL;
}


/**
 * Determines if the given key is present in this map.
 *
 * @param m a pointer to a map, non-NULL
 * @param key a pointer to a key, non-NULL
 * @return true if a key equal to the one pointed to is present in this map,
 * false otherwise
 */
bool gmap_contains_key(const gmap *m, const void *key){
    return gmap_table_find_key(m->table, key, m->hash, m->compare, m->size, m->capacity) < m->size;
}


/**
 * Returns the value associated with the given key in this map.
 * If the key is not present in this map then the returned value is
 * NULL.  The pointer returned is the original pointer passed to gmap_put,
 * and it remains the responsibility of whatever called gmap_put to
 * release the value it points to (no ownership transfer results from
 * gmap_get).
 *
 * @param m a pointer to a map, non-NULL
 * @param key a pointer to a key, non-NULL
 * @return a pointer to the assocated value, or NULL if they key is not present
 */
void *gmap_get(gmap *m, const void *key){
    if (m == NULL || key == NULL){
        return NULL;
    }
    size_t index = gmap_table_find_key(m->table, key, m->hash, m->compare, m->size, m->capacity);
    if(index < m->size){
        return m->table[index].value;
    }
    else{
        return NULL;
    }
}


/**
 * Calls the given function for each (key, value) pair in this map, passing
 * the extra argument as well.
 *
 * @param m a pointer to a map, non-NULL
 * @param f a pointer to a function that takes a key, a value, and an
 * extra piece of information, non-NULL
 * @param arg a pointer
 */
void gmap_for_each(gmap *m, void (*f)(const void *, void *, void *), void *arg){
    if (m == NULL || f == NULL){
        return;
    }
    for (int i = 0; i < m->size; i++){
        f(m->table[i].key, m->table[i].value, arg);
    }
}


/**
 * Returns an array containing pointers to all of the keys in the
 * given map.  The return value is NULL if there was an error
 * allocating the array.  The map retains ownership of the keys, and
 * the pointers to them are only valid as long until they are removed
 * from the map, or until the map is destroyed, whichever comes first.
 * It is the caller's responsibility to destroy the returned array if
 * it is non-NULL.
 *
 * @param m a pointer to a map, non-NULL
 * @return a pointer to an array of pointers to the keys, or NULL
 */
const void **gmap_keys(gmap *m){
    return NULL;
}


/**
 * Destroys the given map.  There is no effect if the given pointer is NULL.
 *
 * @param m a pointer to a map, or NULL
 */
void gmap_destroy(gmap *m){
    return;
}