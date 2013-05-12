/*
 * See Copyright Notice in luci.h
 */

/**
 * @file gc.c
 */

#include "luci.h"
#include "gc.h"


/** global array of pools (one for each allocation size) */
static GCPoolList POOL_LISTS[POOL_LIST_COUNT];

#define GC_MAX_ROOTS 10
/** GC roots array */
static LuciObject** GCRoots[GC_MAX_ROOTS];
static unsigned int gc_num_roots;

bool GC_NECESSARY;
bool GC_UNREACHABLE;
bool GC_REACHABLE;

static GCPool *gc_pool_new(size_t);

/**
 * Initializes the garbage collector and memory-management interface
 *
 * @returns >0 on success, <=0 otherwise
 */
int gc_init(void)
{
    GC_UNREACHABLE = false;
    GC_REACHABLE = true;

    GC_NECESSARY = false;

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

unsigned int gc_add_root(LuciObject **addr)
{
    if (gc_num_roots >= GC_MAX_ROOTS) {
        DIE("%s\n", "Too many roots for gc to track");
    }
    GCRoots[gc_num_roots++] = addr;
    return gc_num_roots;
}

/**
 * Effectively equivalent to system `malloc`.
 * Manages pools of memory for different size-request ranges.
 *
 * @param size size in bytes to allocate
 * @returns void* pointer to allocated block
 */
LuciObject *gc_malloc(LuciObjectType *tp)
{
    /* round size UP to next multiple of a pointer size */
    size_t size = sizeof(void*) * ((tp->size / sizeof(void*)) + 1);

    int idx = size / sizeof(void*) - 1;
    GCPoolList *plist = &POOL_LISTS[idx];

    GCPool *pool = NULL;
    int i;
    for (i = plist->count - 1; i >= 0; i--) {
        pool = plist->pools[i];

        bool full = (pool->next >= (pool->bytes + POOL_SIZE - pool->each));
        if (!full) {
            goto return_object;
        }
    }

    /* time to allocate a new pool */
    if (plist->count >= plist->size) {
        GC_NECESSARY = true;
        plist->size *= 2;
        plist->pools = realloc(plist->pools, plist->size *
                sizeof(*plist->pools));
        if (!plist->pools) {
            DIE("%s\n", "Failed to realloc pool list");
        }
    }

    pool = gc_pool_new(size);
    plist->pools[plist->count++] = pool;

return_object:;
    LuciObject *ret = (LuciObject*)pool->next;
    ret->type = tp;
    ret->reachable = GC_UNREACHABLE;
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
    printf("Marking\n");
    /* mark */
    int i;
    for (i = 0; i < gc_num_roots; i++) {
        LuciObject *obj = *GCRoots[i];
        if (!obj) { DIE("%s\n", "NULL GCRoot"); }
        if (!obj->type) { DIE("%s\n", "NULL GCRoot type pointer"); }
        if (!obj->type->mark) { DIE("%s\n", "NULL GCRoot mark method"); }

        obj->type->mark(obj);
    }

    printf("Sweeping\n");
    /* sweep */
    int swept = 0;
    int unswept = 0;

    int plist_idx;
    for (plist_idx = 0; plist_idx < POOL_LIST_COUNT; plist_idx++) {
        GCPoolList *plist = &POOL_LISTS[plist_idx];
        int pool_idx;
        for (pool_idx = 0; pool_idx < plist->count; pool_idx++) {
            GCPool *pool = plist->pools[pool_idx];
            char *ptr;
            char *limit = (pool->bytes + POOL_SIZE) - pool->each;
            for (ptr = pool->bytes; ptr <= limit; ptr += pool->each) {
                LuciObject *obj = (LuciObject*)ptr;
                if (obj->type != 0) {   /* check that object exists */
                    if (obj->reachable == GC_UNREACHABLE) {
                        if (obj->type->finalize) {
                            obj->type->finalize(obj);
                        }

                        memset(ptr, 0, pool->each);

                        /* update the pool's 'next' pointer if necessary */
                        if (ptr < pool->next) {
                            pool->next = ptr;
                        }
                        swept++;
                    } else {
                        unswept++;
                    }
                }
            }
        }
    }

    /* swap the numeric value of the unreachable flag */
    GC_UNREACHABLE ^= GC_REACHABLE;
    GC_REACHABLE ^= GC_UNREACHABLE;
    GC_UNREACHABLE ^= GC_REACHABLE;

    /* ALWAYS TELL THE WORLD THAT GC IS NO LONGER NECESSARY */
    GC_NECESSARY = false;

    printf("Swept: %d, Unswept: %d\n", swept, unswept);
    return swept;
}

/**
 * Performs housekeeping (memory de-allocation) for the
 * garbage collector and memory-management utilities.
 *
 * @returns 1 on success, <0 otherwise
 */
int gc_finalize()
{
    /* MAYBE replace all this with something like:
     *
     *      GC_REACHABLE = GC_UNREACHABLE;
     *      gc_collect();
     *
     * ??
     */

    int list_idx, pool_idx;
    for (list_idx = 0; list_idx < POOL_LIST_COUNT; list_idx++) {
        GCPoolList *plist = &POOL_LISTS[list_idx];
        for (pool_idx = 0; pool_idx < plist->count; pool_idx++) {
            GCPool *pool = plist->pools[pool_idx];
            char *ptr;
            char *limit = (pool->bytes + POOL_SIZE) - pool->each;
            for (ptr = pool->bytes; ptr <= limit; ptr += pool->each) {
                LuciObject *obj = (LuciObject*)ptr;
                if ((obj->type != 0) && (obj->reachable == GC_UNREACHABLE)) {
                    if (obj->type->finalize) {
                        obj->type->finalize(obj);
                    }
                }
            }
            free(plist->pools[pool_idx]);
        }
        free(plist->pools);
    }


    return 1;
}

/**
 * Allocates a new GCPool
 *
 * @param size the size of each object in the pool
 * @returns new GCPool*
 */
static GCPool *gc_pool_new(size_t size)
{
    GCPool *pool = alloc(sizeof(*pool));
    pool->next = pool->bytes;
    pool->each = size;
    return pool;
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

