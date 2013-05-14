/*
 * See Copyright Notice in luci.h
 */

/**
 * @file constant.h
 */

#ifndef CONSTANT_H
#define CONSTANT_H

#include "lucitypes.h"

/**
 * A table for storing all constants in a Luci program.
 *
 * A constant is either a LuciIntObj, LuciFloatObj, or LuciStringObj.
 * This implementation currently stores all constants linearly
 * in an array of LuciObjects. The plan (TODO:) is to implement
 * a hash table which is capable of hashing any of the constant types
 * in order to deduplicate entries.
 */
typedef struct cotable
{
    LuciObject **objects;   /**< Constant object array */
    uint32_t count;         /**< Current # of objects in array */
    uint32_t size;          /**< Total object array allocated size */
} ConstantTable;

ConstantTable *cotable_new(int size);
void cotable_delete(ConstantTable *);
uint32_t constant_id(ConstantTable *, LuciObject *);

LuciObject **cotable_copy_objects(ConstantTable *);

#endif
