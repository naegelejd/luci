/*
 * See Copyright Notice in luci.h
 */

/**
 * @file map.c
 */

#include <stdlib.h>
#include <string.h>

#include "luci.h"
#include "map.h"

/** Returns the next computed index for the two given hashes
 * @param H0 hash 0
 * @param H1 hash 1
 * @param I  iteration number
 * @param N  table size
 * @returns  index into hash table
 */
#define GET_INDEX(H0, H1, I, N)   ( ( (H0) + ((I) * (I)) * (H1) ) % (N) )


static LuciMapObj *map_grow(LuciMapObj *map);
static LuciMapObj *map_shrink(LuciMapObj *map);
static LuciMapObj *map_resize(LuciMapObj *map, unsigned int new_size);

/** total number of possible hash table sizes */
#define MAX_TABLE_SIZE_OPTIONS  28
/**
 * Array of prime number hash table sizes.
 *
 * Numbers courtesy of:
 * http://planetmath.org/GoodHashTablePrimes.html
 */
static unsigned int table_sizes[] = {
    7, 17, 43, 97, 193, 389, 769, 1543, 3079, 6151,
    12289, 24593, 49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189,
    805306457, 1610612741, 0
};


/** Creates and initializes a new LuciMapObj
 *
 * @returns new LuciMapObj
 */
LuciObject *LuciMap_new()
{
    LuciMapObj *map = alloc(sizeof(*map));
    TYPEOF(map) = obj_map_t;
    map->size_idx = 0;
    map->size = table_sizes[map->size_idx];

    map->keys = alloc(map->size * sizeof(*(map->keys)));
    map->vals = alloc(map->size * sizeof(*(map->vals)));

    return (LuciObject *)map;
}

/*
void map_delete(LuciObject *o)
{
    LuciMapObj *map = AS_MAP(o);
    free(map->keys);
    free(map->vals);
    free(map);
    return;
}
*/

/**
 * Calls map_resize with a larger size index
 *
 * @param o LuciMapObj to enlarge
 * @returns enlarged LuciMapObj
 */
static LuciMapObj *map_grow(LuciMapObj *o)
{
    return map_resize(o, o->size_idx + 1);
}

/**
 * Calls map_resize with a smaller size index
 *
 * @param o LuciMapObj to shrink
 * @returns shrunk LuciMapObj
 */
static LuciMapObj *map_shrink(LuciMapObj *o)
{
    return map_resize(o, o->size_idx + 1);
}

/**
 * Resizes the map's hash table and re-hashes all of its contents.
 *
 * @param map a LuciMapObj
 * @param new_size_idx the new size index used to determine the new size
 * @returns Same LuciMapObj with a new hash table
 */
static LuciMapObj *map_resize(LuciMapObj *map, unsigned int new_size_idx)
{
    int i;
    unsigned int old_size = 0;
    LuciObject **old_keys = NULL;
    LuciObject **old_vals = NULL;

    if (new_size_idx <= 0) {
        return map;
    } else if (new_size_idx > MAX_TABLE_SIZE_OPTIONS) {
        return map;
    }

    old_size = map->size;
    old_keys = map->keys;
    old_vals = map->vals;

    map->size_idx = new_size_idx;
    map->size = table_sizes[new_size_idx];
    map->count = 0;
    map->collisions = 0;

    map->keys = alloc(map->size * sizeof(*(map->keys)));
    map->vals = alloc(map->size * sizeof(*(map->vals)));

    /* re-hash every existing entry into the new, smaller array */
    for (i = 0; i < old_size; i++) {
        map_set((LuciObject *)map, old_keys[i], old_vals[i]);
    }

    free(old_keys);
    free(old_vals);

    return map;
}

/**
 * Sets a key, value pair in the map's hash table
 *
 * @param o     LuciObject (should be a LuciMapObj)
 * @param key   Key to be hashed
 * @param val   Value corresponding to key
 * @returns     original LuciObject with updated hash table
 */
LuciObject *map_set(LuciObject *o, LuciObject *key, LuciObject *val)
{
    if (!o) {
        DIE("%s\n", "Map table not allocated");
    } else if (!key) {
        DIE("%s\n", "Null key in map insertion");
    } else if (TYPEOF(key) != obj_str_t) {
        DIE("%s\n", "Map key must be of type string");
    }

    LuciMapObj *map = AS_MAP(o);

    if (map->count > (map->size * 0.60)) {
        map_grow(map);
    }

    uint32_t hash0 = string_hash_0(key);
    uint32_t hash1 = string_hash_1(key);

    /* otherwise, it's time to search for an empty slot */
    unsigned int i = 0, idx = 0;
    for (i = 0; i < map->size; i++) {
        idx = GET_INDEX(hash0, hash1, i, map->size);

        if (!map->keys[idx]) {
            /* if an empty slot is found, use it and break */
            map->keys[idx] = key;
            map->vals[idx] = val;
            map->count++;
            break;
        } else if (strcmp(
                    AS_STRING(map->keys[idx])->s,
                    AS_STRING(key)->s) == 0) {
            /* TODO: use proper LuciStringObj comparison */
            /* compare objects and break if equal */
            break;
        } else {
            /* just count the collision and continue trying indices */
            map->collisions++;
        }
    }

    /* return the object inserted */
    return key;
}

/**
 * Performs a search for the value corresponding to the given key
 * in the map's hash table
 *
 * @param o     LuciObject (should be a LuciMapObj)
 * @param key   Key to be hashed and searched for
 * @returns     LuciObject value corresponding to key or NULL if not found
 */
LuciObject *map_get(LuciObject *o, LuciObject *key)
{
    if (!o) {
        DIE("%s\n", "Map table not allocated");
    } else if (!key) {
        DIE("%s\n", "Null key in map lookup");
    } else if (TYPEOF(key) != obj_str_t) {
        DIE("%s\n", "Map key must be of type string");
    }

    LuciMapObj *map = AS_MAP(o);

    uint32_t hash0 = string_hash_0(key);
    uint32_t hash1 = string_hash_1(key);

    unsigned int i, idx;
    for (i = 0; i < map->size; i++) {
        idx = GET_INDEX(hash0, hash1, i, map->size);

        if (!map->keys[idx]) {
            /* if we find a NULL slot, it's not in the hash table */
            break;
        } else if (strcmp(
                    AS_STRING(map->keys[idx])->s,
                    AS_STRING(key)->s) == 0) {
            /* TODO: use proper LuciStringObj comparison */
            return map->vals[idx];
        }
    }
    return NULL;
}

/**
 * Performs a search for the value corresponding to the given key
 * in the map's hash table then removes the key,value pair.
 *
 * Once a key is removed, any keys that potentially conflicted with the
 * newly deleted key in the past must be re-hashed in order to preserve
 * the table's integrity for future key insertions.
 *
 * @param o     LuciObject (should be a LuciMapObj)
 * @param key   Key to be hashed and searched for
 * @returns     LuciObject value corresponding to key or NULL if not found
 */
LuciObject *map_remove(LuciObject *o, LuciObject *key)
{
    if (!o) {
        DIE("%s\n", "Map table not allocated");
    } else if (!key) {
        DIE("%s\n", "Null key in map remove");
    } else if (TYPEOF(key) != obj_str_t) {
        DIE("%s\n", "Map key must be of type string");
    }

    LuciMapObj *map = AS_MAP(o);
    LuciObject *val = NULL;

    if (map->count < (map->size * 0.2)) {
        map_shrink(map);
    }

    uint32_t hash0 = string_hash_0(key);
    uint32_t hash1 = string_hash_1(key);

    /* First, find the object to remove */
    unsigned int i, idx;
    for (i = 0; i < map->size; i++) {
        idx = GET_INDEX(hash0, hash1, i, map->size);

        if (!map->keys[idx]) {
            /* if we ever find a null slot, it's not in the table */
            return NULL;
        } else if (strcmp(
                    AS_STRING(map->keys[idx])->s,
                    AS_STRING(key)->s) == 0) {
            /* TODO: use proper LuciStringObj comparison */
            val = map->vals[idx];
            map->count--;
            map->keys[idx] = NULL;
            map->vals[idx] = NULL;
            break;
        } else {
            /* the object at this index possibly needs to be re-inserted */
            ;
        }
    }

    /* now clean up the table */
    LuciObject *key_to_move, *val_to_move;
    for (i = 0; i < map->size; i++) {
        idx = GET_INDEX(hash0, hash1, i, map->size);

        if (!map->keys[idx]) {
            break;
        }
        /* else, delete and reinsert all objects following the
         * newly deleted one. This ensures that all objects are
         * as close as possible to their hash-index on each call
         * to map_set.
         *
         * The alternative method would be to keep track of them
         * in the first loop when searching for the original
         * object to remove */
        else {
            key_to_move = map->keys[idx];
            val_to_move = map->vals[idx];
            map->keys[idx] = NULL;
            map->vals[idx] = NULL;
            map_set((LuciObject *)map, key_to_move, val_to_move);
        }
    }

    return val;
}
