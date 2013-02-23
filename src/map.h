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

LuciObject *map_set(LuciObject *map, LuciObject *key, LuciObject *val);
LuciObject *map_get(LuciObject *map, LuciObject *key);
LuciObject *map_remove(LuciObject *map, LuciObject *key);

#endif /* MAP_H */
