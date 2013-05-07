/*
 * See Copyright Notice in luci.h
 */

/**
 * @file maptype.h
 */

#ifndef LUCI_MAPTYPE_H
#define LUCI_MAPTYPE_H

#include "lucitypes.h"

#define INIT_MAP_SIZE 8     /**< initial allocated size of a map */

extern LuciObjectType obj_map_t;

/** Map object type */
typedef struct LuciMap_ {
    LuciObject base;    /**< base implementation */
    LuciObject **keys;  /**< array of pointers to keys */
    LuciObject **vals;  /**< array of pointers to values */
    unsigned int size_idx;  /**< identifier for current size of table */
    unsigned int collisions;    /**< number of hash collisions */
    unsigned int count;	/**< current number of key/value pairs */
    unsigned int size;	/**< current count of allocated pairs*/
} LuciMapObj;

/** casts LuciObject o to a LuciMapObj */
#define AS_MAP(o)       ((LuciMapObj *)(o))

LuciObject *LuciMap_new();
LuciObject* LuciMap_copy(LuciObject *);
LuciObject* LuciMap_asbool(LuciObject *);
LuciObject* LuciMap_len(LuciObject *);
LuciObject* LuciMap_add(LuciObject *, LuciObject *);
LuciObject* LuciMap_eq(LuciObject *, LuciObject *);
LuciObject* LuciMap_contains(LuciObject *m, LuciObject *o);
LuciObject* LuciMap_next(LuciObject *, LuciObject *);
LuciObject *LuciMap_cput(LuciObject *map, LuciObject *key, LuciObject *val);
LuciObject *LuciMap_cget(LuciObject *map, LuciObject *key);
LuciObject *LuciMap_cdel(LuciObject *map, LuciObject *key);

void LuciMap_print(LuciObject *);


#endif
