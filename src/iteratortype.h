/*
 * See Copyright Notice in luci.h
 */

/**
 * @file iteratortype.h
 */

#ifndef LUCI_ITERATORTYPE_H
#define LUCI_ITERATORTYPE_H

#include "lucitypes.h"

extern LuciObjectType obj_iterator_t;

/** Iterator object type (internal) */
typedef struct LuciIterator_ {
    LuciObject base;        /**< base implemenatation */
    LuciObject *idx;        /**< current index */
    int step;               /**< amount to increment by */
    LuciObject *container;  /**< the container this iterator applies to */
} LuciIteratorObj;

/** casts LuciObject o to a LuciIteratorObj */
#define AS_ITERATOR(o)  ((LuciIteratorObj *)(o))

LuciObject *LuciIterator_new(LuciObject *list, int step);
LuciObject* LuciIterator_copy(LuciObject *);
LuciObject* LuciIterator_asbool(LuciObject *);
void LuciIterator_print(LuciObject *);
void LuciIterator_mark(LuciObject *);

LuciObject *iterator_next_object(LuciObject *iterator);

#endif
