/*
 * See Copyright Notice in luci.h
 */

/**
 * @file iteratortype.c
 */

#include "iteratortype.h"


/** Type member table for LuciIteratorObj */
LuciObjectType obj_iterator_t = {
    "iterator",
    FLAG_DEEP_COPY,
    sizeof(LuciIteratorObj),

    LuciIterator_copy,
    unary_nil,
    LuciIterator_asbool,
    unary_nil,
    unary_nil,
    unary_nil,
    unary_nil,

    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,

    binary_nil,
    binary_nil,
    binary_nil,

    ternary_nil,

    LuciIterator_print,
    LuciIterator_mark,
    NULL,   /* finalize */
    NULL,   /* hash0 */
    NULL    /* hash1 */
};


/**
 * Creates a new LuciIteratorObj
 *
 * @param container container to iterate over
 * @param step iteration increment size
 * @returns new LuciIteratorObj
 */
LuciObject *LuciIterator_new(LuciObject *container, int step)
{
    LuciIteratorObj *o = (LuciIteratorObj*)gc_malloc(&obj_iterator_t);
    o->idx = LuciInt_new(0);
    o->step = step;
    o->container = container;
    return (LuciObject *)o;
}

/**
 * Copies a LuciIteratorObj
 *
 * @param orig LucIteratorObj to copy
 * @returns new copy of orig
 */
LuciObject *LuciIterator_copy(LuciObject *orig)
{
    LuciIteratorObj *iterobj = (LuciIteratorObj *)orig;
    return LuciIterator_new(iterobj->container, iterobj->step);
}

LuciObject* LuciIterator_asbool(LuciObject *o)
{
    LuciObject *res = LuciNilObj;

    LuciObject *container = AS_ITERATOR(o)->container;
    unsigned int len = 0;
    if (ISTYPE(container, obj_list_t)) {
        len = AS_LIST(container)->count;
    } else if (ISTYPE(container, obj_map_t)) {
        len = AS_LIST(container)->size;
    }

    if (AS_INT(AS_ITERATOR(o)->idx)->i < len) {
        res = LuciInt_new(true);
    } else {
        res = LuciInt_new(false);
    }
    return res;
}

/**
 * Returns the next LuciObject in a container.
 *
 * @param iterator from which to compute next object
 * @returns next object in iterator's sequence or NULL if finished iterating
 */
LuciObject *iterator_next_object(LuciObject *iterator)
{
    if (!iterator || (!ISTYPE(iterator, obj_iterator_t))) {
        DIE("%s", "Can't get next from non-iterator object\n");
    }

    LuciIteratorObj *iter = (LuciIteratorObj *)iterator;
    LuciObject *container = iter->container;

    LuciObject *next = container->type->next(container, iter->idx);
    AS_INT(iter->idx)->i += iter->step;
    return next;
}

void LuciIterator_print(LuciObject *in)
{
    printf("<iterator>");
}

void LuciIterator_mark(LuciObject *in)
{
    LuciObject *idx = AS_ITERATOR(in)->idx;
    LuciObject *container = AS_ITERATOR(in)->container;

    idx->type->mark(idx);
    container->type->mark(container);

    GC_MARK(in);
}
