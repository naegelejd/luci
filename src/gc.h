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

#define POOL_LIST_COUNT  10     /**< number of different pool lists available */
#define INIT_POOL_LIST_SIZE 4   /**< initial size of pool list array */
#define POOL_SIZE  6144         /**< initial pool size in bytes */

/** marks an object as reachable via root objects */
#define GC_MARK(obj)    (obj)->reachable = GC_REACHABLE;

/** identifies whether garbage collection is necessary */
extern bool GC_NECESSARY;
/** defines an object as unreachable from GC roots */
extern bool GC_UNREACHABLE;
/** defines an object as reachable from GC roots */
extern bool GC_REACHABLE;

/** used only for initializing static LuciObjects */
#define GC_STATIC 2

/** Wraps gc_collect, calling it only if garbage collection is necessary */
#define GC_COLLECT() do { if (GC_NECESSARY) gc_collect(); } while (0)

/** Contains pointers to a byte array for use by the memory pool,
 * as well as a large bit-flag (implemented using an array
 * of unsigned ints) used for tri-color marking of objects in the pool */
typedef struct pool_ {
    size_t each;        /**< size of each object in pool */
    char *next;         /**< pointer to next free object */
    char bytes[POOL_SIZE];  /**< block of memory in which objects are stored */
    bool full;          /**< whether the pool is full of objects */
} GCPool;

/** Dynamic list of memory pools for LuciObjects */
typedef struct pool_list_ {
    GCPool **pools;     /**< array of pools */
    unsigned int size;  /**< allocated size of pools array */
    unsigned int count; /**< current # of pools */
} GCPoolList;

/** Dynamic list for root LuciObjects */
typedef struct roots_list_ {
    LuciObject ***roots;    /**< array of root objects */
    unsigned int size;      /**< allocated size of array */
    unsigned int count;     /**< current # of roots */
} GCRootList;

int gc_init(void);
LuciObject *gc_malloc(LuciObjectType *);
int gc_collect(void);
int gc_finalize(void);

void gc_track_root(LuciObject **root);
void gc_untrack_roots(void);


#endif /* LUCI_GC_H */
