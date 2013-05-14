/*
 * See Copyright Notice in luci.h
 */

/**
 * @file constant.c
 */

#include "luci.h"
#include "constant.h"
#include "lucitypes.h"


/**
 * Allocates and initializes a new constant table.
 *
 * @param size initial capacity of the table's object array.
 * @returns new ConstantTable\*
 */
ConstantTable *cotable_new(int size)
{
    ConstantTable *cotable = alloc(sizeof(*cotable));
    cotable->count = 0;
    cotable->size = size;
    cotable->objects = alloc(cotable->size * sizeof(*cotable->objects));
    return cotable;
}

/**
 * Frees all memory allocated for constant table
 *
 * @param cotable the ConstantTable to delete
 */
void cotable_delete(ConstantTable *cotable)
{
    free(cotable->objects);
    cotable->objects = NULL;
    free(cotable);
    return;
}

/**
 * Returns the ID of the constant value.
 * Inserts object into table if it is not already present
 *
 * @param cotable the ConstantTable in which to insert the object
 * @param const_obj the object to insert
 * @returns the "ID" of the constant
 */
uint32_t constant_id(ConstantTable *cotable, LuciObject *const_obj)
{
    if (const_obj == NULL)
        DIE("%s", "Can't index a NULL constant\n");

    if (cotable->count > cotable->size) {
        cotable->size <<= 1;
        cotable->objects = realloc(cotable->objects,
                cotable->size * sizeof(*cotable->objects));
    }
    /* store object */
    cotable->objects[cotable->count] = const_obj;
    /* return count (index a.k.a. ID ) */
    return cotable->count++;
}

/**
 * Returns a copy of the constant table's objects array.
 *
 * @param cotable the ConstantTable containing the desired object array
 * @returns copy of the array of LuciObject constants
 */
LuciObject **cotable_copy_objects(ConstantTable *cotable)
{
    if (!cotable) {
        DIE("%s\n", "Cannot get object array from NULL ConstantTable\n");
    }

    size_t bytes = cotable->count * sizeof(*cotable->objects);
    LuciObject **copy = alloc(bytes);
    memcpy(copy, cotable->objects, bytes);

    return copy;
}
