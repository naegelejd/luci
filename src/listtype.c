/*
 * See Copyright Notice in luci.h
 */

/**
 * @file listtype.c
 */

#include "listtype.h"


static LuciObject *list_get_object(LuciObject *list, long index);
static LuciObject *list_set_object(LuciObject *list, LuciObject *item, long index);

/** Type member table for LuciListObj */
LuciObjectType obj_list_t = {
    "list",
    FLAG_SHALLOW_COPY,
    sizeof(LuciListObj),

    LuciList_copy,
    unary_nil,
    LuciList_asbool,
    LuciList_len,
    unary_nil,
    unary_nil,
    unary_nil,

    LuciList_add,   /* add */
    binary_nil,     /* sub */
    binary_nil,     /* mul */
    binary_nil,     /* div */
    binary_nil,     /* mod */
    binary_nil,     /* pow */
    LuciList_eq,    /* eq */
    binary_nil,     /* neq */
    LuciList_append,     /* lt */
    binary_nil,     /* gt */
    binary_nil,     /* lte */
    binary_nil,     /* gte */
    binary_nil,     /* lgor */
    binary_nil,     /* lgand */
    binary_nil,     /* bwxor */
    binary_nil,     /* bwor */
    binary_nil,     /* bwand */

    LuciList_contains,
    LuciList_next,
    LuciList_cget,

    LuciList_cput,

    LuciList_print
};

/**
 * Creates a new, empty LuciListObj
 *
 * @returns new empty LuciListObj
 */
LuciObject *LuciList_new()
{
    LuciListObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_list_t);
    o->count = 0;
    o->size = INIT_LIST_SIZE;
    o->items = alloc(o->size * sizeof(*o->items));
    return (LuciObject *)o;
}

/**
 * Returns a COPY of the object in the list at the index
 *
 * @param list LuciListObj to grab from
 * @param index index from which to grab object
 * @returns copy of LuciObject at index
 */
static LuciObject *list_get_object(LuciObject *list, long index)
{
    if (!list || (!ISTYPE(list, obj_list_t))) {
	DIE("%s", "Can't iterate over non-list object\n");
    }

    LuciListObj *listobj = (LuciListObj *)list;

    /* convert negative indices to a index starting from list end */
    while (index < 0) {
	index = listobj->count - abs(index);
    }

    if (index >= listobj->count) {
	DIE("%s", "List index out of bounds\n");
    }
    LuciObject *item = listobj->items[index];
    return item->type->copy(item);
}

/**
 * Assigns a LuciObject to a LuciListObj at a given index.
 *
 * @param list LuciListObj to modify
 * @param item LuciObj to insert into the list
 * @param index index at which to store object
 * @returns the object that formerly resided at index
 */
static LuciObject *list_set_object(LuciObject *list, LuciObject *item, long index)
{
    if (!list || (!ISTYPE(list, obj_list_t))) {
	DIE("%s", "Can't iterate over non-list object\n");
    } else if (!item) {
	DIE("%s", "NULL item in list assignment\n");
    }

    LuciListObj *listobj = (LuciListObj *)list;

    while (index < 0) {
	index = listobj->count = abs(index);
    }
    /* list_get_object will take care of any more error handling */
    LuciObject *old = list_get_object(list, index);
    listobj->items[index] = item;
    return old;
}

/**
 * Copies a LuciListObj
 *
 * @param orig LucListObj to copy
 * @returns new copy of orig
 */
LuciObject* LuciList_copy(LuciObject *orig)
{
    LuciListObj *listobj = (LuciListObj *)orig;
    int i;

    LuciObject *copy = LuciList_new();

    for (i = 0; i < listobj->count; i++) {
        LuciList_append(copy, list_get_object(orig, i));
    }

    return copy;
}

LuciObject* LuciList_asbool(LuciObject *o)
{
    return LuciInt_new(AS_LIST(o)->count > 0);
}

/**
 * Returns the length of a LuciListObj
 *
 * @param o LuciListObj
 * @returns length of o
 */
LuciObject* LuciList_len(LuciObject *o)
{
    return LuciInt_new(AS_LIST(o)->count);
}


/**
 * Concatenates two LuciListObjs
 *
 * @param a first LuciListObj
 * @param b second LuciListObj
 * @returns concatenated LuciListObj
 */
LuciObject* LuciList_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_list_t)) {
        res = LuciList_new();
        int i;
        for (i = 0; i < AS_LIST(a)->count; i++) {
            LuciList_append(res, AS_LIST(a)->items[i]);
        }
        for (i = 0; i < AS_LIST(b)->count; i++) {
            LuciList_append(res, AS_LIST(b)->items[i]);
        }
    } else {
        DIE("Cannot append object of type %s to a list\n",
                b->type->type_name);
    }

    return res;
}

/**
 * Appends an object to a LuciListObj
 *
 * @param l list
 * @param b item to append
 * @returns LuciNilObj
 */
LuciObject* LuciList_append(LuciObject *l, LuciObject *b)
{
    LuciListObj *list = AS_LIST(l);

    if (list->count >= list->size) {
	list->size = list->size << 1;
	/* realloc the list array */
	list->items = realloc(list->items,
		list->size * sizeof(*list->items));
        if (!list->items) {
            DIE("%s", "Failed to dynamically expand list while appending\n");
        }
	LUCI_DEBUG("%s\n", "Reallocated space for list");
    }
    /* increment count after appending object */
    list->items[list->count++] = b;

    return LuciNilObj;
}

/**
 * Pops the tail item off the LuciListObj
 *
 * @param l list
 * @returns former tail of list
 */
LuciObject* LuciList_pop(LuciObject *l)
{
    LuciListObj *list = AS_LIST(l);

    list->count--;
    if (list->count < (list->size / 2)) {
        list->size = list->size >> 1;
        list->items = realloc(list->items,
                list->size * sizeof(*list->items));
        if (!list->items) {
            DIE("%s\n", "Failed to dynamically shrink list while popping");
        }
        LUCI_DEBUG("%s\n", "Shrunk list");
    }

    LuciObject *ret = list->items[list->count];
    list->items[list->count] = NULL;
    return ret;
}

/**
 * Returns the tail item of the LuciListObj
 *
 * @param l list
 * @returns tail of list
 */
LuciObject* LuciList_tail(LuciObject *l)
{
    return AS_LIST(l)->items[AS_LIST(l)->count - 1];
}

/**
 * Determines if two LuciListObjs are equal
 *
 * @param a LuciListObj
 * @param b LuciListObj
 * @returns 1 if equal, 0 otherwise
 */
LuciObject* LuciList_eq(LuciObject *a, LuciObject *b)
{
    if(ISTYPE(b, obj_list_t)) {
        if (AS_LIST(a)->count != AS_LIST(b)->count) {
            return LuciInt_new(false);
        }
        int i;
        for (i = 0; i < AS_LIST(a)->count; i++) {
            LuciObject *item1 = AS_LIST(a)->items[i];
            LuciObject *item2 = AS_LIST(b)->items[i];
            LuciObject *eq = item1->type->eq(item1, item2);
            /* if the objects in the lists at index i aren't equal,
             * return false */
            if (!AS_INT(eq)->i) {
                return LuciInt_new(false);
            }
        }
        /* all objects match */
        return LuciInt_new(true);
    } else {
        DIE("Cannot compare a list to an object of type %s\n",
                b->type->type_name);
    }
    return LuciNilObj;
}


/**
 * Determines whether a LuciListObj contains an object
 *
 * @param l LuciListObj
 * @param o object
 * @returns 1 if str contains o, 0 otherwise
 */
LuciObject *LuciList_contains(LuciObject *l, LuciObject *o)
{
    int i;
    for (i = 0; i < AS_LIST(l)->count; i++) {
        LuciObject *x = AS_LIST(l)->items[i];
        LuciObject *eq = o->type->eq(o, x);
        if (AS_INT(eq)->i) {
            return LuciInt_new(true);
        }
    }
    return LuciInt_new(false);
}

/**
 * Returns the 'next' object in the list
 *
 * @param l LuciListObj
 * @param idx index
 * @returns copy of object at index idx or NULL if out of bounds
 */
LuciObject *LuciList_next(LuciObject *l, LuciObject *idx)
{
    if (!ISTYPE(idx, obj_int_t)) {
        DIE("%s\n", "Argument to LuciList_next must be LuciIntObj");
    }

    if (AS_INT(idx)->i >= AS_LIST(l)->count) {
        return NULL;
    }

    LuciObject *item = AS_LIST(l)->items[AS_INT(idx)->i];
    return item->type->copy(item);
}

/**
 * Gets the object at index b in LuciListObj a
 *
 * @param a LuciListObj
 * @param b index in a
 * @returns object at index b
 */
LuciObject* LuciList_cget(LuciObject *a, LuciObject *b)
{
    if (ISTYPE(b, obj_int_t)) {
        return list_get_object(a, AS_INT(b)->i);
    } else {
        DIE("Cannot subscript a list with an object of type %s\n",
                b->type->type_name);
    }

    return LuciNilObj;
}

LuciObject* LuciList_cput(LuciObject *a, LuciObject *b, LuciObject *c)
{
    if (ISTYPE(b, obj_int_t)) {
        return list_set_object(a, c, AS_INT(b)->i);
    } else {
        DIE("Cannot subscript a list with an object of type %s\n",
                b->type->type_name);
    }

    return LuciNilObj;
}


/**
 * Prints a LuciListObj to stdout
 *
 * @param in LuciListObj to print
 */
void LuciList_print(LuciObject *in)
{
    int i;
    printf("[");
    for (i = 0; i < AS_LIST(in)->count; i++) {
        LuciObject *item = list_get_object(in, i);
        item->type->print(item);
        printf(", ");
    }
    printf("]");
}
