/*
 * See Copyright Notice in luci.h
 */

/**
 * @file gc.h
 */

#ifndef LUCI_GC_H
#define LUCI_GC_H

#include "luci.h"
#include "lucitypes.h"

#define POOL_LIST_COUNT  10     /**< number of pools available */
#define INIT_POOL_LIST_SIZE 4
#define POOL_SIZE  6144    /**< initial pool size in bytes */

#define GC_MARK(obj)    (obj)->reachable = GC_REACHABLE;

extern bool GC_NECESSARY;
extern bool GC_UNREACHABLE;
extern bool GC_REACHABLE;
#define GC_STATIC 2

#define GC_COLLECT() do { if (GC_NECESSARY) gc_collect(); } while (0)

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
unsigned int gc_add_root(LuciObject **);
LuciObject *gc_malloc(LuciObjectType *);
int gc_collect(void);
int gc_finalize(void);

#endif /* LUCI_GC_H */
