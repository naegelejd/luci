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

static GCRootList gc_roots = { NULL, 0, 0 };

#define POOL_LIMIT(pool)    ((pool)->bytes + POOL_SIZE - (pool)->each)

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

    /* initialize GC roots list */
    gc_roots.count = 0;
    gc_roots.size = 2;
    gc_roots.roots = alloc(gc_roots.size * sizeof(*gc_roots.roots));

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
 * Adds an address to a pointer to a LuciObject to the GC Roots list
 *
 * Dynamically expands the GC roots list as necessary
 *
 * @param root Address of a pointer to a LuciObject (e.g. main stack)
 */
void gc_track_root(LuciObject **root)
{
    if (gc_roots.count > gc_roots.size) {
        gc_roots.size *= 2;
        gc_roots.roots = realloc(gc_roots.roots,
                gc_roots.size * sizeof(*gc_roots.roots));
        if (!gc_roots.roots) {
            DIE("%s\n", "Failed to realloc GC Roots list");
        }
    }

    gc_roots.roots[gc_roots.count++] = root;
}

/**
 * Removes the address of a pointer to a LuciObject from the GC Roots list
 *
 * Does not currently shrink the GC roots list as necessary
 *
 * @param root Address to pointer to LuciObject (e.g. main stack)
 */
void gc_untrack_root(LuciObject **root)
{
    unsigned int i;
    for (i = 0; i < gc_roots.count; i++) {
        if (gc_roots.roots[i] == root) {
            gc_roots.roots[i] = NULL;
        }
    }
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

    unsigned int idx = size / sizeof(void*) - 1;
    GCPoolList *plist = &POOL_LISTS[idx];

    int i;
    for (i = plist->count - 1; i >= 0; i--) {
    //for (i = 0; i < plist->count; i++) {
        GCPool *pool = plist->pools[i];

        if (pool->next) {
            LuciObject *ret = (LuciObject*)pool->next;
            ret->type = tp;
            ret->reachable = GC_UNREACHABLE;

            while (true) {
                pool->next += pool->each;

                if (pool->next > POOL_LIMIT(pool)) {
                    //printf("Pool full: %p > (%p + %u - %lu)\n",
                            //pool->next, pool->bytes, POOL_SIZE, pool->each);
                    pool->next = NULL;
                    break;
                }

                LuciObject *obj = (LuciObject *)pool->next;
                if (obj->type == 0) {
                    break;
                }
            }

            return ret;
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

    GCPool *pool = gc_pool_new(size);
    plist->pools[plist->count++] = pool;
    LuciObject *ret = (LuciObject *)pool->next;
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
    LUCI_DEBUG("%s\n", "GC Marking");
    unsigned int i;
    for (i = 0; i < gc_roots.count; i++) {
        LuciObject **root_addr = gc_roots.roots[i];
        if (root_addr) {
            /* NOTE: this will segfault if a non-heap-allocated root goes
             * out of scope before `gc_collect` is called */
            LuciObject *root = *root_addr;
            if (root) {
                root->type->mark(root);
            }
        }
    }


    LUCI_DEBUG("%s\n", "GC Sweeping");
    int swept = 0;
    int unswept = 0;

    /* goto skip_sweep; */

    unsigned int plist_idx;
    for (plist_idx = 0; plist_idx < POOL_LIST_COUNT; plist_idx++) {
        GCPoolList *plist = &POOL_LISTS[plist_idx];
        unsigned int pool_idx;
        for (pool_idx = 0; pool_idx < plist->count; pool_idx++) {
            GCPool *pool = plist->pools[pool_idx];
            char *ptr;
            for (ptr = pool->bytes; ptr <= POOL_LIMIT(pool); ptr += pool->each) {
                LuciObject *obj = (LuciObject*)ptr;
                if (obj->type != 0) {   /* check that object exists */
                    if (obj->reachable == GC_UNREACHABLE) {
                        if (obj->type->finalize) {
                            obj->type->finalize(obj);
                        }

                        ptr = memset(ptr, 0, pool->each);

                        /* update the pool's 'next' pointer if necessary */
                        if (!pool->next || (ptr < pool->next)) {
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

skip_sweep:

    /* swap the numeric value of the unreachable flag */
    GC_UNREACHABLE ^= GC_REACHABLE;
    GC_REACHABLE ^= GC_UNREACHABLE;
    GC_UNREACHABLE ^= GC_REACHABLE;

    /* ALWAYS TELL THE WORLD THAT GC IS NO LONGER NECESSARY */
    GC_NECESSARY = false;

    LUCI_DEBUG("Swept: %d, Unswept: %d\n", swept, unswept);
    return swept;
}

/**
 * Performs housekeeping (memory de-allocation) for the
 * garbage collector and memory-management utilities.
 *
 * @returns number of objects finalized
 */
int gc_finalize()
{
    unsigned int finalized = 0;
    unsigned int list_idx, pool_idx;
    for (list_idx = 0; list_idx < POOL_LIST_COUNT; list_idx++) {
        GCPoolList *plist = &POOL_LISTS[list_idx];
        for (pool_idx = 0; pool_idx < plist->count; pool_idx++) {
            GCPool *pool = plist->pools[pool_idx];
            char *ptr;
            for (ptr = pool->bytes; ptr <= POOL_LIMIT(pool); ptr += pool->each) {
                LuciObject *obj = (LuciObject*)ptr;
                if (obj->type != 0) {
                    if (obj->type->finalize) {
                        obj->type->finalize(obj);
                        finalized++;
                    }
                }
            }
            free(plist->pools[pool_idx]);
        }
        free(plist->pools);
    }

    LUCI_DEBUG("Finalized %u objects\n", finalized);
    return finalized;
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

