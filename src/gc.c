/*
 * See Copyright Notice in luci.h
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "luci.h"
#include "gc.h"

//#include <sys/time.h>
//struct timeval t1, t2;

#define INIT_ARENA_POOLS 1

#define ARENA_COUNT 6
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
        ARENA[i].pool_total = INIT_ARENA_POOLS << 4;
        ARENA[i].pools = malloc(sizeof(*(ARENA[i].pools)) * ARENA[i].pool_total);
        if (!ARENA[i].pools) {
            DIE("%s", "Failed to allocate Arena pool pointer array\n");
        }
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

    if (size < 8) {
        return NULL;
    }

    arena = &ARENA[(size >> 3) - 1];

    /* if this is the first request for this size, allocate pool(s) */
    if (arena->pool_count == 0) {
        arena->pool_count = INIT_ARENA_POOLS;
        for (i = 0; i < arena->pool_count; i++) {
            arena->pools[i] = gc_pool_new();
        }
    }

//    gettimeofday(&t1, NULL);
    /* only `POOL_SIZE / size / 32` flags are being used in each pool */
    unsigned int num_flags = POOL_SIZE / (size << 5);
    unsigned int r;
    /* for now, I'm just starting at the newest pools and searching backwards.
     * the pool pointers should be moved dynamically within the Arena's array
     * to account for their measure of emptiness */
    for (i = arena->pool_count - 1; i >= 0; i--) {
        pool = arena->pools[i];
        for (j = 0; j < num_flags; j++) {
//printf("%d Checking flag: %lX\n", j, (long unsigned int)pool->used_flags[j]);
            if (pool->used_flags[j] < 0xFFFFFFFFL) {
                /* found a pool with an available slot somewhere */

                r = 0;
                while (pool->used_flags[j] & (1L << r)) {
                    r++;
                }

//printf("%lX\n", ~(1L << r));

                /* mark this slot as taken (set bit) */
                pool->used_flags[j] |= (1L << r);

                /* addr is pool + offset where,
                 * offset = flagnumber * flagbits * size + r * size
                 */
                void *addr = (void *)pool + (((j << 5) + r) * size);
//printf("%p\n", pool);
//printf("%p\n", addr);
//printf("(%d << 6 + %d) * %lu == %lu ?\n", j, r, size, addr-(void *)pool);
                return addr;
                /* return (void *)(pool + ((j << 6) + r) * size); */
            }
        }
    }
//    gettimeofday(&t2, NULL);
//printf("%u.%06u\n", (t2.tv_sec - t1.tv_sec), (t2.tv_usec - t1.tv_usec));

//gettimeofday(&t1, NULL);
//printf("%s", "Allocating new pool\n");

    /* didn't find open slot, add a pool to arena */
    /* first, expand the arena's pool pointer array if necessary */
    if (arena->pool_count >= arena->pool_total) {
        /* the arena pool pointer array is now at least pool_count + 1 */
        arena->pool_total <<= 1;
//printf("%u %u\n", arena->pool_count, arena->pool_total);
        arena->pools = realloc(arena->pools, sizeof(*(arena->pools)) * arena->pool_total);
        if (!arena->pools) {
            DIE("%s", "Failed to realloc arena.pools array\n");
        }
    }

    /* allocate pool */
    pool = gc_pool_new();
    /* set first bit of first flag (slot no longer free) */
    pool->used_flags[0] |= 1;
    arena->pools[arena->pool_count] = pool;
    arena->pool_count++;

//gettimeofday(&t2, NULL);
//printf("%u.%06u\n", (t2.tv_sec - t1.tv_sec), (t2.tv_usec - t1.tv_usec));

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
//printf("Arena for size %u had %u pools\n", (i+1)<<3, ARENA[i].pool_count);
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

