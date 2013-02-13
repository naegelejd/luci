/*
 * See Copyright Notice in luci.h
 */

#ifndef MAP_H
#define MAP_H

#include "object.h"

typedef struct {
    unsigned char flags;
    unsigned char size_idx; /* Index of current table size */
    unsigned int count;     /* Current # of hashed objects */
    unsigned int collisions;/* Current # of hash collisions */
    unsigned int size;      /* Current # of objects memory has been allocated for */
    LuciObject **keys;   /* Array of LuciObjects keys */
    LuciObject **vals;   /* Array of LuciObjects vals */
} MapTable;


LuciObject *LuciMap_new();
/* void map_delete(LuciObject *map); */

LuciObject *map_set(LuciObject *map, LuciObject *key, LuciObject *val);
LuciObject *map_get(LuciObject *map, LuciObject *key);
LuciObject *map_remove(LuciObject *map, LuciObject *key);

#endif /* MAP_H */
