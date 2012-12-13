/*
 * See Copyright Notice in luci.h
 */

#include <stdlib.h>
#include <string.h>

#include "luci.h"
#include "object.h"
#include "constant.h"


ConstantTable *cotable_new(int size)
{
    ConstantTable *cotable = alloc(sizeof(*cotable));
    cotable->count = 0;
    cotable->size = size;
    cotable->objects = alloc(cotable->size * sizeof(*cotable->objects));
    cotable->owns_objects = 1;
    return cotable;
}

/**
 * Frees all memory allocated for constant table
 */
void cotable_delete(ConstantTable *cotable)
{
    /* only deallocate objects array if cotable still has ownership */
    if (cotable->owns_objects > 0) {
        /* destroy all constant objects */
        int i;
        for (i = 0; i < cotable->count; i ++)
            destroy(cotable->objects[i]);
        free(cotable->objects);
        cotable->objects = NULL;
    }
    free(cotable);
    cotable = NULL;

    return;
}

/**
 * Returns the ID of the constant value
 * Inserts object into table if it is not already present
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

LuciObject **cotable_get_objects(ConstantTable *cotable)
{
    if (!cotable) {
        return NULL;
    }

    cotable->owns_objects = 0;

    return cotable->objects;
}
