/*
 * See Copyright Notice in luci.h
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "luci.h"
#include "gc.h"


#define INIT_ARENA_POOLS 1

#define ARENA_COUNT 5
static struct gc_arena ARENA[ARENA_COUNT];

static struct gc_pool * gc_pool_new(void);
static void gc_pool_delete(struct gc_pool *);


/**
 * Initializes the garbage collector and memory-management interface
 */
int gc_init(void)
{
    int i, j;

    /* initialize each arena identifying it as empty */
    for (i = 0; i < ARENA_COUNT; i++) {
        ARENA[i].pool_count = 0;
        ARENA[i].pool_total = 0;
        ARENA[i].pools = NULL;
    }

    return 1;
}

/**
 * Effectively equivalent to system `malloc`.
 * Manages pools of memory for different size-request ranges.
 */
void * gc_malloc(size_t size)
{
    int i, j;
    struct gc_arena *arena = NULL;
    struct gc_pool *pool = NULL;

    arena = &ARENA[(size >> 3) - 1];

    /* if this Arena's pools haven't been created... do so on the first
     * request for this size */
    if (arena->pools == NULL) {
        printf("Allocating arena for pool size %lu\n", size);
        arena->pool_count = INIT_ARENA_POOLS;
        arena->pool_total = INIT_ARENA_POOLS;
        arena->pools = malloc(sizeof(*(arena->pools)) * arena->pool_total);
        if (!arena->pools) {
            DIE("Failed to malloc pools array (arena for size %lu)\n", size);
        }
        for (j = 0; j < arena->pool_count; j++) {
            arena->pools[j] = gc_pool_new();
        }
    }

    /* only `POOL_SIZE / size / 64` flags are being used in each pool */
    unsigned int num_flags = POOL_SIZE / (size << 6);
    unsigned int r = 0;
    uint64_t tmp = 0;
    for (i = 0; i < arena->pool_count; i++) {
        pool = arena->pools[i];
        for (j = 0; j < num_flags; j++) {
//printf("%d Checking flag: %lX\n", j, (long unsigned int)pool->free_flags[j]);
            if (pool->free_flags[j] > 0) {
                /* found a pool with an available slot somewhere */
                tmp = pool->free_flags[j];

                while (tmp >>= 1) {
                    r++;
                }

                /* mark this slot as taken */
                pool->free_flags[j] &= ~(1L << r);

                /* addr is pool + offset where,
                 * offset = flagnumber * flagbits * size + r * size
                 */
                void *addr = (void *)pool + (((j << 6) + r) * size);
//printf("%p\n", pool);
//printf("%p\n", addr);
//printf("(%d << 6 + %d) * %lu == %lu ?\n", j, r, size, addr-(void *)pool);
                return addr;
                /* return (void *)(pool + ((j << 6) + r) * size); */
            }
        }
    }

//printf("%s", "Allocating new pool\n");
    /* didn't find open slot, add a pool to arena */
    /* first, add and possibly expand the arena's pool pointer array */
    arena->pool_count++;
    if (arena->pool_count > arena->pool_total) {
        arena->pool_total <<= 1;
//printf("%u %u\n", arena->pool_count, arena->pool_total);
        arena->pools = realloc(arena->pools, sizeof(*(arena->pools)) * arena->pool_total);
        if (!arena->pools) {
            DIE("%s", "Failed to realloc arena.pools array\n");
        }
    }

    /* allocate pool */
    pool = gc_pool_new();
    /* set first bit of all flags to zero (slot no longer free) */
    pool->free_flags[0] &= (uint64_t)(-1) - 1;
    arena->pools[arena->pool_count - 1] = pool;
    /* return first slot in pool */
    return (void *)pool;
}

/**
 * Effectively equivalent to system `free`.
 * Gives allocated slots of memory back to their respective pool
 */
void gc_free(void *ptr)
{

}

/**
 * Performs housekeeping (memory de-allocation) for the
 * garbage collector and memory-management utilities.
 */
int gc_finalize()
{
    int i, j;
    for (i = 0; i < ARENA_COUNT; i++) {
        printf("Arena for size %u had %u pools\n", (i+1)<<3, ARENA[i].pool_count);
        for (j = 0; j < ARENA[i].pool_count; j++) {
            gc_pool_delete(ARENA[i].pools[j]);
        }
        free(ARENA[i].pools);
    }

    return 1;
}

static struct gc_pool * gc_pool_new(void)
{
    int i;
    /* calloc so that both the blocks returned to the user are
     * zero'd, as well as the flags used to identify used/free blocks */
    struct gc_pool *p = calloc(1, sizeof(*p));
    if (!p) {
        DIE("%s", "Failed to calloc a new gc_pool\n");
    }
    /* initialize each flags to 64 '1' bits each */
    for (i = 0; i < NUM_FLAGS; i++) {
        p->free_flags[i] = (uint64_t)(-1);
    }
    return p;
}

static void gc_pool_delete(struct gc_pool *p)
{
    free(p);
}


/*
 * Generic calloc wrapper
 *
 * Allocates and zeros memory for all
 * non-LuciObject requests.
 */
void *alloc(size_t size)
{
    void *result = calloc(size, 1);
    if (!result) {
	DIE("%s", "alloc failed\n");
    }
    return result;
}

