/*
 * See Copyright Notice in luci.h
 */
#include <stdlib.h>
#include <string.h>

#include "luci.h"
#include "map.h"

#define GET_INDEX(H0, H1, I, N)   ( ( (H0) + ((I) * (I)) * (H1) ) % (N) )

static MapTable *mt_grow(MapTable *mt);
static MapTable *mt_shrink(MapTable *mt);
static MapTable *mt_resize(MapTable *mt, unsigned int new_size);


#define MAX_TABLE_SIZE_OPTIONS  28
/* http://planetmath.org/GoodHashTablePrimes.html */
static unsigned int table_sizes[] = {
    7, 17, 43, 97, 193, 389, 769, 1543, 3079, 6151,
    12289, 24593, 49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189,
    805306457, 1610612741, 0
};


MapTable *mt_new()
{
    MapTable *mt = calloc(1, sizeof(*mt));
    if (!mt) {
        DIE("%s\n", "Error allocating map table");
    }

    mt->size_idx = 0;
    mt->size = table_sizes[mt->size_idx];

    mt->keys = calloc(mt->size, sizeof(*(mt->keys)));
    if (!mt->keys) {
        DIE("%s\n", "Error allocating map's keys array");
    }

    mt->vals = calloc(mt->size, sizeof(*(mt->vals)));
    if (!mt->vals) {
        DIE("%s\n", "Error allocating map's vals array");
    }

    return mt;
}

void mt_delete(MapTable *mt)
{
    free(mt->keys);
    free(mt->vals);
    free(mt);
    return;
}

static MapTable *mt_grow(MapTable *mt)
{
    return mt_resize(mt, mt->size_idx + 1);
}

static MapTable *mt_shrink(MapTable *mt)
{
    return mt_resize(mt, mt->size_idx + 1);
}

static MapTable *mt_resize(MapTable *mt, unsigned int new_size_idx)
{
    int i;
    unsigned int old_size = 0;
    LuciObject **old_keys = NULL;
    LuciObject **old_vals = NULL;

    if (new_size_idx <= 0) {
        return mt;
    } else if (new_size_idx > MAX_TABLE_SIZE_OPTIONS) {
        return mt;
    }

    old_size = mt->size;
    old_keys = mt->keys;
    old_vals = mt->vals;

    mt->size_idx = new_size_idx;
    mt->size = table_sizes[new_size_idx];
    mt->count = 0;
    mt->collisions = 0;

    mt->keys = calloc(mt->size, sizeof(*(mt->keys)));
    if (!mt->keys) {
        DIE("%s\n", "Error resizing map's keys array");
    }

    mt->vals = calloc(mt->size, sizeof(*(mt->vals)));
    if (!mt->vals) {
        DIE("%s\n", "Error resizing map's vals array");
    }

    /* re-hash every existing entry into the new, smaller array */
    for (i = 0; i < old_size; i++) {
        mt_insert(mt, old_keys[i], old_vals[i]);
    }

    free(old_keys);
    free(old_vals);

    return mt;
}

LuciObject *mt_insert(MapTable *mt, LuciObject *key, LuciObject *val)
{
    if (!mt) {
        DIE("%s\n", "Map table not allocated");
    } else if (!key) {
        DIE("%s\n", "Null key in map insertion");
    } else if (TYPEOF(key) != obj_str_t) {
        DIE("%s\n", "Map key must be of type string");
    }

    if (mt->count > (mt->size * 0.60)) {
        mt_grow(mt);
    }

    uint32_t hash0 = string_hash_0(key);
    uint32_t hash1 = string_hash_1(key);

    /* otherwise, it's time to search for an empty slot */
    unsigned int i = 0, idx = 0;
    for (i = 0; i < mt->size; i++) {
        idx = GET_INDEX(hash0, hash1, i, mt->size);

        if (!mt->keys[idx]) {
            /* if an empty slot is found, use it and break */
            mt->keys[idx] = key;
            mt->vals[idx] = val;
            mt->count++;
            break;
        } else if (strcmp(
                    AS_STRING(mt->keys[idx])->s,
                    AS_STRING(key)->s) == 0) {
            /* TODO: use proper LuciStringObj comparison */
            /* compare objects and break if equal */
            break;
        } else {
            /* just count the collision and continue trying indices */
            mt->collisions++;
        }
    }

    /* return the object inserted */
    return key;
}

LuciObject *mt_lookup(MapTable *mt, LuciObject *key)
{
    if (!mt) {
        DIE("%s\n", "Map table not allocated");
    } else if (!key) {
        DIE("%s\n", "Null key in map lookup");
    } else if (TYPEOF(key) != obj_str_t) {
        DIE("%s\n", "Map key must be of type string");
    }

    uint32_t hash0 = string_hash_0(key);
    uint32_t hash1 = string_hash_1(key);

    unsigned int i, idx;
    for (i = 0; i < mt->size; i++) {
        idx = GET_INDEX(hash0, hash1, i, mt->size);

        if (!mt->keys[idx]) {
            /* if we find a NULL slot, it's not in the hash table */
            break;
        } else if (strcmp(
                    AS_STRING(mt->keys[idx])->s,
                    AS_STRING(key)->s) == 0) {
            /* TODO: use proper LuciStringObj comparison */
            return mt->vals[idx];
        }
    }
    return NULL;
}

LuciObject *mt_remove(MapTable *mt, LuciObject *key)
{
    if (!mt) {
        DIE("%s\n", "Map table not allocated");
    } else if (!key) {
        DIE("%s\n", "Null key in map remove");
    } else if (TYPEOF(key) != obj_str_t) {
        DIE("%s\n", "Map key must be of type string");
    }

    if (mt->count < (mt->size * 0.2)) {
        mt_shrink(mt);
    }

    uint32_t hash0 = string_hash_0(key);
    uint32_t hash1 = string_hash_1(key);

    /* First, find the object to remove */
    unsigned int i, idx;
    for (i = 0; i < mt->size; i++) {
        idx = GET_INDEX(hash0, hash1, i, mt->size);

        if (!mt->keys[idx]) {
            /* if we ever find a null slot, it's not in the table */
            return NULL;
        } else if (strcmp(
                    AS_STRING(mt->keys[idx])->s,
                    AS_STRING(key)->s) == 0) {
            /* TODO: use proper LuciStringObj comparison */
            mt->count--;
            mt->keys[idx] = NULL;
            mt->vals[idx] = NULL;
            break;
        } else {
            /* the object at this index possibly needs to be re-inserted */
            ;
        }
    }

    /* now clean up the table */
    LuciObject *key_to_move, *val_to_move;
    for (i = 0; i < mt->size; i++) {
        idx = GET_INDEX(hash0, hash1, i, mt->size);

        if (!mt->keys[idx]) {
            break;
        }
        /* else, delete and reinsert all objects following the
         * newly deleted one. This ensures that all objects are
         * as close as possible to their hash-index on each call
         * to mt_insert.
         *
         * The alternative method would be to keep track of them
         * in the first loop when searching for the original
         * object to remove */
        else {
            key_to_move = mt->keys[idx];
            val_to_move = mt->vals[idx];
            mt->keys[idx] = NULL;
            mt->vals[idx] = NULL;
            mt_insert(mt, key_to_move, val_to_move);
        }
    }

    return key;
}
