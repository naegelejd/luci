/*
 * See Copyright Notice in luci.h
 */
#include <stdlib.h>
#include <string.h>

#include "luci.h"
#include "map.h"

#define GET_INDEX(H0, H1, I, N)   ( ( (H0) + ((I) * (I)) * (H1) ) % (N) )

static LuciMapObj *map_grow(LuciMapObj *map);
static LuciMapObj *map_shrink(LuciMapObj *map);
static LuciMapObj *map_resize(LuciMapObj *map, unsigned int new_size);


#define MAX_TABLE_SIZE_OPTIONS  28
/* http://planetmath.org/GoodHashTablePrimes.html */
static unsigned int table_sizes[] = {
    7, 17, 43, 97, 193, 389, 769, 1543, 3079, 6151,
    12289, 24593, 49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189,
    805306457, 1610612741, 0
};


LuciObject *LuciMap_new()
{
    LuciMapObj *map = alloc(sizeof(*map));
    REFCOUNT(map) = 0;
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

static LuciMapObj *map_grow(LuciMapObj *o)
{
    return map_resize(o, o->size_idx + 1);
}

static LuciMapObj *map_shrink(LuciMapObj *o)
{
    return map_resize(o, o->size_idx + 1);
}

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

    return key;
}
