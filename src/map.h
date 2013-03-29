/*
 * See Copyright Notice in luci.h
 */

/**
 * @file map.h
 *
 */

#ifndef MAP_H
#define MAP_H

#include "object.h"

LuciObject *LuciMap_new();
/* void map_delete(LuciObject *map); */

LuciObject *LuciMap_cput(LuciObject *map, LuciObject *key, LuciObject *val);
LuciObject *LuciMap_cget(LuciObject *map, LuciObject *key);
LuciObject *LuciMap_cdel(LuciObject *map, LuciObject *key);

#endif /* MAP_H */
