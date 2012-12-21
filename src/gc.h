/*
 * See Copyright Notice in luci.h
 */
#ifndef LUCI_GC_H
#define LUCI_GC_H

#include <stdlib.h>
#include <stdint.h>


//#define POOL_SIZE 8000
//#define POOL_SIZE 4096
#define POOL_SIZE 6144

/* the number of uint64 flags must be (POOL_SIZE / 8 / 64)
 * because the smallest allocation is 8 bytes */
//#define NUM_FLAGS 8
#define NUM_FLAGS 24


struct gc_pool {
    char pool[POOL_SIZE];
    uint32_t used_flags[NUM_FLAGS];
};

struct gc_arena {
    struct gc_pool **pools;
    unsigned int pool_count;
    unsigned int pool_total;
};

int gc_init(void);
void * gc_malloc(size_t);
void gc_free(void *);
int gc_finalize(void);

#endif /* LUCI_MALLOC_H */
