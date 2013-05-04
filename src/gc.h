/*
 * See Copyright Notice in luci.h
 */

/**
 * @file gc.h
 */

#ifndef LUCI_GC_H
#define LUCI_GC_H

#include <stdlib.h>
#include <stdint.h>


//#define POOL_SIZE 8000
//#define POOL_SIZE 4096
#define POOL_SIZE 6144  /**< initial pool size in bytes */

/* the number of uint32 flags must be (POOL_SIZE / 8 / 32)
 * because the smallest allocation is 8 bytes */
#define NUM_FLAGS 24    /**< = (POOL_SIZE / 8 / 32) */

/** Contains a byte array for use by the memory pool,
 * as well as a large bit-flag (implemented using an array
 * of unsigned ints) used for marking sections of the pool
 * as used or available */
struct gc_pool {
    char pool[POOL_SIZE];       /**< space for use by pool manager */
    uint32_t used_flags[NUM_FLAGS]; /**< 1 bit for each block in the pool */
};

/** Parent of gc_pool. Contains an array of memory pools
 * and tracks how many are allocated/used. */
struct gc_arena {
    struct gc_pool **pools;     /**< array of gc_pool pointers */
    unsigned int pool_count;    /**< current number of gc_pools being used */
    unsigned int pool_total;    /**< total number of gc_pools allocated */
};

struct pool_ {
    size_t size;
    size_t block;
    char *base;
    char *next;
    uint32_t *colors;
};

int gc_init(void);
void * gc_malloc(size_t);
int gc_finalize(void);

#endif /* LUCI_MALLOC_H */
