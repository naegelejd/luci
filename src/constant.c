/*
 * See Copyright Notice in luci.h
 */

#include "stdlib.h"
#include "common.h"
#include "object.h"
#include "constant.h"


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
 */
void cotable_delete(ConstantTable *cotable)
{
    int i;
    /* destroy all constant objects */
    for (i = 0; i < cotable->count; i ++)
        destroy(cotable->objects[i]);
    free(cotable->objects);
    cotable->objects = NULL;
    free(cotable);
    cotable = NULL;

    return ;
}

/**
 * Returns the ID of the constant value
 * Inserts object into table if it is not already present
 */
int constant_id(ConstantTable *cotable, LuciObject *const_obj)
{
    if (const_obj == NULL)
        die("Can't index a NULL constant\n");

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
 * Returns a COPY of the constant object
 */
LuciObject *cotable_get(ConstantTable *cotable, int id)
{
    if ((id < 0) || (id >= cotable->count))
        die("Constant id out of bounds\n");

    /* return a copy of the constant */
    return copy_object(cotable->objects[id]);
}
