/*
 * See Copyright Notice in luci.h
 */

/**
 * @file gc.h
 */

#ifndef LUCI_GC_H
#define LUCI_GC_H

#include "luci.h"

#define POOL_LIST_COUNT  10     /**< number of pools available */
#define INIT_POOL_LIST_SIZE 4
#define POOL_SIZE  6144    /**< initial pool size in bytes */


/** Contains pointers to a byte array for use by the memory pool,
 * as well as a large bit-flag (implemented using an array
 * of unsigned ints) used for tri-color marking of objects in the pool */
typedef struct pool_ {
    size_t each;        /**< size of each object in pool */
    char *next;         /**< pointer to next free object */
    char bytes[POOL_SIZE];
} GCPool;

typedef struct pool_list_ {
    GCPool **pools;
    unsigned int size;
    unsigned int count;
} GCPoolList;

int gc_init(void);
void *gc_malloc(size_t);
int gc_collect(void);
int gc_finalize(void);

#endif /* LUCI_MALLOC_H */
