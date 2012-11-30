/*
 * See Copyright Notice in luci.h
 */

#ifndef CONSTANT_H
#define CONSTANT_H

#include <stdint.h>
#include "object.h"

typedef struct cotable
{
    uint8_t owns_objects; /* boolean. Affects deallocation of Table */
    uint32_t count;      /* Current # of objects in array */
    uint32_t size;       /* Total object array allocated size */
    LuciObject **objects;   /* Constant object array */
} ConstantTable;

ConstantTable *cotable_new();
void cotable_delete(ConstantTable *);
uint32_t constant_id(ConstantTable *, LuciObject *);

LuciObject **cotable_get_objects(ConstantTable *);

#endif
