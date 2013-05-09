/*
 * See Copyright Notice in luci.h
 */

/**
 * @file gc.c
 */

#include "luci.h"
#include "gc.h"
#include "lucitypes.h"

//#include <sys/time.h>
//struct timeval t1, t2;

/** global array of arenas (one for each allocation size) */
static GCPoolList POOL_LISTS[POOL_LIST_COUNT];

static GCPool *gc_pool_new(size_t);


static GCPool *gc_pool_new(size_t size)
{
    GCPool *pool = alloc(sizeof(*pool));
    pool->next = pool->bytes;
    pool->each = size;
    return pool;
}


/**
 * Initializes the garbage collector and memory-management interface
 *
 * @returns >0 on success, <=0 otherwise
 */
int gc_init(void)
{
    /* initialize each arena identifying it as empty */
    int i;
    for (i = 0; i < POOL_LIST_COUNT; i++) {
        GCPoolList *plist = &POOL_LISTS[i];
        plist->count = 0;
        plist->size = INIT_POOL_LIST_SIZE;
        plist->pools = alloc(plist->size * sizeof(*plist->pools));
    }

    return POOL_LIST_COUNT;
}

/**
 * Effectively equivalent to system `malloc`.
 * Manages pools of memory for different size-request ranges.
 *
 * @param size size in bytes to allocate
 * @returns void* pointer to allocated block
 */
void *gc_malloc(size_t size)
{
    /* round size UP to next multiple of a pointer size */
    size = sizeof(void*) * ((size / sizeof(void*)) + 1);

    int idx = size / sizeof(void*) - 1;
    GCPoolList *plist = &POOL_LISTS[idx];

    int i;
    for (i = plist->count - 1; i >= 0; i--) {
        GCPool *pool = plist->pools[i];

        bool full = (pool->next >= (pool->bytes + POOL_SIZE - pool->each));
        if (!full) {
            void *ret = pool->next;
            pool->next += pool->each;
            return ret;
        }
    }

    /* time to allocate a new pool */
    if (plist->count >= plist->size) {
        plist->size *= 2;
        plist->pools = realloc(plist->pools, plist->size *
                sizeof(*plist->pools));
        if (!plist->pools) {
            DIE("%s\n", "Failed to realloc pool list");
        }
    }

    GCPool *pool = gc_pool_new(size);
    plist->pools[plist->count++] = pool;

    void *ret = pool->next;
    pool->next += pool->each;
    return ret;
}


/**
 * Collects unreachable LuciObjects for reuse.
 *
 * @returns count of objects recycled
 */
int gc_collect(void)
{
    return 0;
}

/**
 * Performs housekeeping (memory de-allocation) for the
 * garbage collector and memory-management utilities.
 *
 * @returns 1 on success, <0 otherwise
 */
int gc_finalize()
{
    int list_idx, pool_idx;
    for (list_idx = 0; list_idx < POOL_LIST_COUNT; list_idx++) {
        GCPoolList *plist = &POOL_LISTS[list_idx];
        for (pool_idx = 0; pool_idx < plist->count; pool_idx++) {
            free(plist->pools[pool_idx]);
        }
        free(plist->pools);
    }

    return 1;
}


/**
 * Generic calloc wrapper
 *
 * Allocates and zeros memory for all
 * non-LuciObject requests.
 *
 * @param size size in bytes to allocate
 * @returns void* pointer to allocated block
 */
void *alloc(size_t size)
{
    void *result = calloc(size, 1);
    if (!result) {
	DIE("%s", "alloc failed\n");
    }
    return result;
}

