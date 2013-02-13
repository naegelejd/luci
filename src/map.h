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


MapTable *mt_new();
void mt_delete(MapTable *mt);

LuciObject *mt_insert(MapTable *mt, LuciObject *key, LuciObject *val);
LuciObject *mt_lookup(MapTable *mt, LuciObject *key);
LuciObject *mt_remove(MapTable *mt, LuciObject *key);

#endif /* MAP_H */
