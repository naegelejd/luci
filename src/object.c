/*
 * See Copyright Notice in luci.h
 */

/**
 * @file object.c
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "luci.h"
#include "gc.h"
#include "object.h"
#include "map.h"

/* temporary */
#include "compile.h" /* for destroying function object FOR NOW */

static LuciObject* LuciInt_copy(LuciObject *);
static LuciObject* LuciFloat_copy(LuciObject *);
static LuciObject* LuciString_copy(LuciObject *);
static LuciObject* LuciList_copy(LuciObject *);
static LuciObject* LuciMap_copy(LuciObject *);
static LuciObject* LuciIterator_copy(LuciObject *);
static LuciObject* LuciFunction_copy(LuciObject *);
static LuciObject* LuciLibFunc_copy(LuciObject *);

static LuciObject* LuciInt_repr(LuciObject *);
static LuciObject* LuciFloat_repr(LuciObject *);
static LuciObject* LuciString_repr(LuciObject *);

static LuciObject* LuciInt_add(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_add(LuciObject *, LuciObject *);
static LuciObject* LuciString_add(LuciObject *, LuciObject *);
static LuciObject* LuciList_add(LuciObject *, LuciObject *);
static LuciObject* LuciMap_add(LuciObject *, LuciObject *);

static void LuciInt_print(LuciObject *);
static void LuciFloat_print(LuciObject *);
static void LuciString_print(LuciObject *);
static void LuciFile_print(LuciObject *);
static void LuciList_print(LuciObject *);
static void LuciMap_print(LuciObject *);
static void LuciFile_print(LuciObject *);
static void LuciFunction_print(LuciObject *in);
static void LuciLibFunc_print(LuciObject *in);


/** Type member table for LuciIntObj */
LuciObjectType obj_int_t = {
    "int",
    LuciInt_copy,
    LuciInt_repr,
    LuciInt_add,
    LuciInt_print
};

/** Type member table for LuciFloatObj */
LuciObjectType obj_float_t = {
    "float",
    LuciFloat_copy,
    LuciFloat_repr,
    LuciFloat_add,
    LuciFloat_print
};

/** Type member table for LuciStringObj */
LuciObjectType obj_string_t = {
    "string",
    LuciString_copy,
    LuciString_repr,
    LuciString_add,
    LuciString_print
};

/** Type member table for LuciListObj */
LuciObjectType obj_list_t = {
    "list",
    LuciList_copy,
    unary_null,
    LuciList_add,
    LuciList_print
};

/** Type member table for LuciMapObj */
LuciObjectType obj_map_t = {
    "map",
    LuciMap_copy,
    unary_null,
    LuciMap_add,
    LuciMap_print
};

/** Type member table for LuciFileObj */
LuciObjectType obj_file_t = {
    "file",
    unary_null,
    unary_null,
    binary_null,
    LuciFile_print
};

/** Type member table for LuciIteratorObj */
LuciObjectType obj_iterator_t = {
    "iterator",
    LuciIterator_copy,
    unary_null,
    binary_null,
    unary_noop
};

/** Type member table for LuciFunctionObj */
LuciObjectType obj_func_t = {
    "function",
    LuciFunction_copy,
    unary_null,
    binary_null,
    LuciFunction_print
};

/** Type member table for LuciLibFuncObj */
LuciObjectType obj_libfunc_t = {
    "libfunction",
    LuciLibFunc_copy,
    unary_null,
    binary_null,
    LuciLibFunc_print
};

/**
 * Creates a new LuciIntObj
 *
 * @param l long integer value
 * @returns new LuciIntObj
 */
LuciObject *LuciInt_new(long l)
{
    LuciIntObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_int_t);
    o->i = l;
    return (LuciObject *)o;
}

/**
 * Creates a new LuciFloatObj
 *
 * @param d double floating-point value
 * @returns new LuciFloatObj
 */
LuciObject *LuciFloat_new(double d)
{
    LuciFloatObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_float_t);
    o->f = d;
    return (LuciObject *)o;
}

/**
 * Creates a new LuciStringObj
 *
 * @param s C-string value
 * @returns new LuciStringObj
 */
LuciObject *LuciString_new(char *s)
{
    LuciStringObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_string_t);
    o->s = s;   /* not a copy! */
    o->len = strlen(o->s);
    return (LuciObject *)o;
}

/**
 * Creates a new LuciFileObj
 *
 * @param fp C file pointer
 * @param size length of file
 * @param mode enumerated file mode
 * @returns new LuciFileObj
 */
LuciObject *LuciFile_new(FILE *fp, long size, file_mode mode)
{
    LuciFileObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_file_t);
    o->ptr = fp;
    o->mode = mode;
    o->size = size;
    return (LuciObject *)o;
}

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
 * Creates a new LuciIteratorObj
 *
 * @param container container to iterate over
 * @param step iteration increment size
 * @returns new LuciIteratorObj
 */
LuciObject *LuciIterator_new(LuciObject *container, unsigned int step)
{
    LuciIteratorObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_iterator_t);
    o->idx = 0;
    o->step = step;
    o->container = container;
    return (LuciObject *)o;
}

/**
 * Creates a new LuciFunctionObj
 *
 * @param frame Frame struct defining function
 * @returns new LuciFunctionObj
 */
LuciObject *LuciFunction_new(void *frame)
{
    LuciFunctionObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_func_t);
    o->frame = frame;
    return (LuciObject *)o;
}

/**
 * Creates a new LuciLibFuncObj
 *
 * @param func C function pointer matching LuciLibFunc signature
 * @returns new LuciLibFuncObj
 */
LuciObject *LuciLibFunc_new(LuciObject * (*func)(LuciObject **, unsigned int))
{
    LuciLibFuncObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_libfunc_t);
    o->func = func;
    return (LuciObject *)o;
}

/**
 * Deallocates and destroys a LuciObject.
 *
 * @param trash LuciObject to destroy
 */
/*
void destroy(LuciObject *trash)
{
    if (!trash) {
        LUCI_DEBUG("%s\n", "Destroying a NULL object...");
        return;
    }

    int i;
    switch(trash->type) {

        case obj_file_t:
            if (((LuciFileObj *)trash)->ptr) {
                // TODO: method of closing only open files
                // LUCI_DEBUG("%s\n", "Closing file object");
                // close_file(trash->value.file.ptr);
            }
            break;

        case obj_string_t:
            LUCI_DEBUG("Freeing string %s\n", ((LuciStringObj *)trash)->s);
            free(((LuciStringObj *)trash)->s);
            ((LuciStringObj *)trash)->s = NULL;
            break;

        case obj_list_t:
        {
            LuciListObj *listobj = (LuciListObj *)trash;
            for (i = 0; i < listobj->count; i++) {
                destroy(listobj->items[i]);
            }
            free(listobj->items);
            break;
        }

        case obj_map_t:
        {
            LuciMapObj *mapobj = (LuciMapObj *)trash;
            for (i = 0; i < mapobj->size; i++) {
                if (mapobj->keys[i]) {
                    destroy(mapobj->keys[i]);
                    destroy(mapobj->vals[i]);
                }
            }
            free(mapobj->keys);
            free(mapobj->vals);
            break;
        }

        case obj_iterator_t:
            // destroy the list if possible
            destroy(((LuciIteratorObj *)trash)->container);
            break;

        case obj_func_t:
            Frame_delete(((LuciFunctionObj *)trash)->frame);
            break;

        default:
            break;
    }
    LUCI_DEBUG("Destroying obj @ %lu with type %d\n",
            (unsigned long) trash, trash->type);

    // destroy the LuciObject itself
    gc_free(trash);
    trash = NULL;
}
*/

/**
 * Computes a hash of a LuciStringObj
 *
 * djb2 (hash(i) = hash(i - 1) * 33 + str[i])
 *
 * @param s LuciStringObj to hash
 * @returns unsigned integer hash
 */
unsigned int string_hash_0(LuciObject *s)
{
    char *str = AS_STRING(s)->s;

    unsigned int h = 5381;
    int c;

    while ((c = *str++))
        h = ((h << 5) + h) + c;
    return h;
}

/**
 * Computes a hash of a LuciStringObj
 *
 * sdbm (hash(i) = hash(i - 1) * 65599 + str[i])
 *
 * @param s LuciStringObj to hash
 * @returns unsigned integer hash
 */
unsigned int string_hash_1(LuciObject *s)
{
    char *str = AS_STRING(s)->s;

    unsigned int h = 0;
    int c;
    while ((c = *str++))
        h = c + (h << 6) + (h << 16) - h;
    return h;
}

/**
 * Computes a hash of a LuciStringObj
 *
 * One-at-a-time (Bob Jenkins)
 *
 * @param s LuciStringObj to hash
 * @returns unsigned integer hash
 */
unsigned int string_hash_2(LuciObject *s)
{
    char *str = AS_STRING(s)->s;

    unsigned int h = 0;
    int c;
    while ((c = *str++)) {
        h += c;
        h += h << 10;
        h ^= h >> 6;
    }
    h += h << 3;
    h ^= h >> 11;
    h += h << 15;
    return h;
}

/**
 * Appends a LuciObject to a LuciListObj
 *
 * @param list LuciListObj to append to
 * @param item LuciObject to append to the list
 * @returns 1 on success, 0 otherwise
 */
int list_append_object(LuciObject *list, LuciObject *item)
{
    LuciListObj *listobj = NULL;

    if (!list || (!ISTYPE(list, obj_list_t))) {
	DIE("%s", "Can't append item to non-list object\n");
    }

    listobj = (LuciListObj *)list;

    if (listobj->count >= listobj->size) {
	listobj->size = listobj->size << 1;
	/* realloc the list array */
	listobj->items = realloc(listobj->items,
		listobj->size * sizeof(*listobj->items));
        if (!listobj->items) {
            DIE("%s", "Failed to dynamically expand list while appending\n");
        }
	LUCI_DEBUG("%s\n", "Reallocated space for list");
    }
    /* increment count after appending object */
    listobj->items[listobj->count++] = item;
    return 1;
}

/**
 * Returns a COPY of the object in the list at the index
 *
 * @param list LuciListObj to grab from
 * @param index index from which to grab object
 * @returns copy of LuciObject at index
 */
LuciObject *list_get_object(LuciObject *list, int index)
{
    LuciListObj *listobj = NULL;

    if (!list || (!ISTYPE(list, obj_list_t))) {
	DIE("%s", "Can't iterate over non-list object\n");
    }

    listobj = (LuciListObj *)list;

    /* convert negative indices to a index starting from list end */
    while (index < 0) {
	index = listobj->count - abs(index);
    }

    if (index >= listobj->count) {
	DIE("%s", "List index out of bounds\n");
	/* return NULL; */
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
LuciObject *list_set_object(LuciObject *list, LuciObject *item, int index)
{
    LuciListObj *listobj = NULL;

    if (!list || (!ISTYPE(list, obj_list_t))) {
	DIE("%s", "Can't iterate over non-list object\n");
    }
    if (!item) {
	DIE("%s", "Can't set list item to NULL\n");
    }

    listobj = (LuciListObj *)list;

    while (index < 0) {
	index = listobj->count = abs(index);
    }
    /* list_get_object will take care of any more error handling */
    LuciObject *old = list_get_object(list, index);
    listobj->items[index] = item;
    return old;
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

    if (ISTYPE(iter->container, obj_list_t)) {
        LuciListObj *list = (LuciListObj *)iter->container;
        uint32_t idx = iter->idx;   /* save current index */

        if (iter->idx >= list->count) {
            return NULL;
        } else {
            iter->idx += iter->step;
            LuciObject *item = list->items[idx];
            return item->type->copy(item);
        }
    } else if (ISTYPE(iter->container, obj_map_t)) {
        LuciMapObj *map = (LuciMapObj *)iter->container;

        if (iter->idx >= map->size) {
            return NULL;
        }

        if (iter->idx == 0) {
            /* be sure we're at a valid 'index' in the map's hash table */
            while ((iter->idx < map->size) &&
                    (map->keys[iter->idx] == NULL)) {
                iter->idx += 1;
            }
        }

        uint32_t idx = iter->idx;   /* save this valid index */
        do {
            /* increment the index for the next call */
            iter->idx += iter->step;
            /* this will leave the idx at either a valid key
             * or the last index in the map's key array */
        } while ((iter->idx < map->size) &&
                (map->keys[iter->idx] == NULL));
        LuciObject *key = map->keys[idx];
        return key->type->copy(key);
    } else {
        DIE("%s\n", "Cannot iterate over a non-container type");
    }
    return NULL;
}

/**
 * Prints a LuciObject to stdout.
 *
 * @param in LuciObject to print
 */
/*
void print_object(LuciObject *in)
{
    int i;
    LuciObject *item;

    if (!in) {
	printf("None");
	return;
    }

    switch (in->type) {
	case obj_int_t:
	    printf("%ld", AS_INT(in)->i);
	    break;

	case obj_float_t:
	    printf("%f", AS_FLOAT(in)->f);
	    break;

	case obj_string_t:
	    printf("%s", AS_STRING(in)->s);
	    break;

	case obj_list_t:
	    printf("[");
	    for (i = 0; i < AS_LIST(in)->count; i++) {
		item = list_get_object(in, i);
		print_object(item);
		printf(", ");
	    }
	    printf("]");
	    break;

        case obj_map_t:
            printf("{");
            for (i = 0; i < AS_MAP(in)->size; i++) {
                if (AS_MAP(in)->keys[i]) {
                    printf("\"");
                    print_object(AS_MAP(in)->keys[i]);
                    printf("\":");
                    print_object(AS_MAP(in)->vals[i]);
                    printf(", ");
                }
            }
            printf("}");
            break;

        case obj_file_t:
            printf("<file>");
            break;

        case obj_func_t:
            printf("<func>");
            break;

        case obj_libfunc_t:
            printf("<libfunc>");
            break;

	default:
            DIE("%s\n", "Can't print invalid type");
    }
}
*/


/**
 * Performs a deep copy of a LuciObject.
 *
 * @param orig LuciObject to copy
 * @returns deep copy of orig
 */
/*
LuciObject *copy_object(LuciObject *orig)
{
    LuciObject *copy = NULL;

    if (!orig) {
	return NULL;
    }

    switch(orig->type)
    {
	case obj_int_t:
            copy = LuciInt_new(((LuciIntObj *)orig)->i);
	    break;

	case obj_float_t:
            copy = LuciFloat_new(((LuciFloatObj *)orig)->f);
	    break;

	case obj_string_t:
            // duplicate string first
            copy = LuciString_new(strdup(((LuciStringObj *)orig)->s));
	    break;

        case obj_file_t:
        {
            LuciFileObj *fileobj = (LuciFileObj *)orig;
            copy = LuciFile_new(fileobj->ptr, fileobj->size, fileobj->mode);
	    break;
        }

	case obj_list_t:
        {
            LuciListObj *listobj = (LuciListObj *)orig;
            int i;

            copy = LuciList_new();

	    for (i = 0; i < listobj->count; i++) {
		list_append_object(copy, list_get_object(orig, i));
	    }
	    break;
        }

        case obj_map_t:
        {
            LuciMapObj *mapobj = (LuciMapObj *)orig;
            int i;

            copy = LuciMap_new();

            for (i = 0; i < mapobj->size; i++) {
                if (mapobj->keys[i]) {
                    map_set(copy,
                            copy_object(mapobj->keys[i]),
                            copy_object(mapobj->vals[i]));
                }
            }
            break;
        }

        case obj_iterator_t:
        {
            LuciIteratorObj *iterobj = (LuciIteratorObj *)orig;
            copy = LuciIterator_new(iterobj->container, iterobj->step);
            break;
        }

        case obj_func_t:
            copy = LuciFunction_new(((LuciFunctionObj *)orig)->frame);
            break;

        case obj_libfunc_t:
            copy = LuciLibFunc_new(((LuciLibFuncObj *)orig)->func);
            break;

	default:
            ;
    }

    return copy;
}
*/


/**
 * Copies a LuciIntObj
 *
 * @param orig LucIntObj to copy
 * @returns new copy of orig
 */
static LuciObject* LuciInt_copy(LuciObject *orig)
{
    return LuciInt_new(((LuciIntObj *)orig)->i);
}

/**
 * Copies a LuciFloatObj
 *
 * @param orig LucFloatObj to copy
 * @returns new copy of orig
 */
static LuciObject* LuciFloat_copy(LuciObject *orig)
{
    return LuciFloat_new(((LuciFloatObj *)orig)->f);
}

/**
 * Copies a LuciStringObj
 *
 * @param orig LucStringObj to copy
 * @returns new copy of orig
 */
static LuciObject* LuciString_copy(LuciObject *orig)
{
    return LuciString_new(strdup(((LuciStringObj *)orig)->s));
}

/**
 * Copies a LuciListObj
 *
 * @param orig LucListObj to copy
 * @returns new copy of orig
 */
static LuciObject* LuciList_copy(LuciObject *orig)
{
    LuciListObj *listobj = (LuciListObj *)orig;
    int i;

    LuciObject *copy = LuciList_new();

    for (i = 0; i < listobj->count; i++) {
        list_append_object(copy, list_get_object(orig, i));
    }

    return copy;
}

/**
 * Copies a LuciMapObj
 *
 * @param orig LucMapObj to copy
 * @returns new copy of orig
 */
static LuciObject *LuciMap_copy(LuciObject *orig)
{
    LuciMapObj *mapobj = (LuciMapObj *)orig;
    int i;

    LuciObject *copy = LuciMap_new();

    for (i = 0; i < mapobj->size; i++) {
        if (mapobj->keys[i]) {
            LuciObject *key = mapobj->keys[i];
            LuciObject *val = mapobj->vals[i];
            map_set(copy, key->type->copy(key), val->type->copy(val));
        }
    }
    return copy;
}

/**
 * Copies a LuciIteratorObj
 *
 * @param orig LucIteratorObj to copy
 * @returns new copy of orig
 */
static LuciObject *LuciIterator_copy(LuciObject *orig)
{
    LuciIteratorObj *iterobj = (LuciIteratorObj *)orig;
    return LuciIterator_new(iterobj->container, iterobj->step);
}

/**
 * Copies a LuciFunctionObj
 *
 * @param orig LucFunctionObj to copy
 * @returns new copy of orig
 */
static LuciObject *LuciFunction_copy(LuciObject *orig)
{
    return LuciFunction_new(((LuciFunctionObj *)orig)->frame);
}

/**
 * Copies a LuciLibFuncObj
 *
 * @param orig LucLibFuncObj to copy
 * @returns new copy of orig
 */
static LuciObject *LuciLibFunc_copy(LuciObject *orig)
{
    return LuciLibFunc_new(((LuciLibFuncObj *)orig)->func);
}


/**
 * Produces the LuciStringObj representation of a LuciIntObj
 *
 * @param o LuciIntObj to represent
 * @returns LuciStringObj representation of o
 */
static LuciObject* LuciInt_repr(LuciObject *o)
{
    char *s = alloc(MAX_INT_DIGITS);
    snprintf(s, MAX_INT_DIGITS, "%ld", AS_INT(o)->i);
    /* ret->s[16] = '\0'; */
    return LuciString_new(s);
}

/**
 * Produces the LuciStringObj representation of a LuciFloatObj
 *
 * @param o LuciFloatObj to represent
 * @returns LuciStringObj representation of o
 */
static LuciObject* LuciFloat_repr(LuciObject *o)
{
    char *s = alloc(MAX_FLOAT_DIGITS);
    snprintf(s, MAX_FLOAT_DIGITS, "%f", (float)AS_FLOAT(o)->f);
    /* AS_STRING(ret)->s[16] = '\0'; */
    return LuciString_new(s);
}

/**
 * Produces the LuciStringObj representation of a LuciStringObj
 *
 * While this may seem redundant, cast a string to a string
 * must return a *new* string.
 *
 * @param o LuciStringObj to represent
 * @returns LuciStringObj representation of o
 */
static LuciObject* LuciString_repr(LuciObject *o)
{
    int len = AS_STRING(o)->len + 1;
    char *s = alloc(len);
    strncpy(s, AS_STRING(o)->s, len);
    return LuciString_new(s);
}


/**
 * Adds a LuciIntObj to a second numeric LuciObject
 *
 * If b is a LuciFloatObj, the sum is promoted to a LuciFloatObj
 *
 * @param a LuciIntObj
 * @param b LuciIntObj or LuciFloatObj
 * @returns sum as a LuciIntObj or LuciFloatObj
 */
static LuciObject* LuciInt_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = NULL;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i + AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i + AS_FLOAT(b)->f);
    } else {
        DIE("Cannot add object of type %s to an int\n", b->type->type_name);
    }
    return res;
}

/**
 * Adds a LuciFloatObj to a second numeric LuciObject
 *
 * @param a LuciFloatObj
 * @param b LuciIntObj or LuciFloatObj
 * @returns sum as a LuciFloatObj
 */
static LuciObject* LuciFloat_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = NULL;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f + AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f + AS_FLOAT(b)->f);
    } else {
        DIE("Cannot add object of type %s to a float\n", b->type->type_name);
    }
    return res;
}

/**
 * Concatenates two LuciStringObjs
 *
 * @param a first LuciStringObj
 * @param b second LuciStringObj
 * @returns concatenated LuciStringObj
 */
static LuciObject* LuciString_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = NULL;

    if (ISTYPE(b, obj_string_t)) {
        char *s = alloc(AS_STRING(a)->len + AS_STRING(b)-> len + 1);
        strncpy(s, AS_STRING(a)->s, AS_STRING(a)->len);
        strncat(s, AS_STRING(b)->s, AS_STRING(b)->len);
        res = LuciString_new(s);
    } else {
        DIE("Cannot append object of type %s to a string\n",
                b->type->type_name);
    }

    return res;
}

/**
 * Concatenates two LuciListObjs
 *
 * @param a first LuciListObj
 * @param b second LuciListObj
 * @returns concatenated LuciListObj
 */
static LuciObject* LuciList_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = NULL;

    if (ISTYPE(b, obj_list_t)) {
        res = LuciList_new();
        int i;
        for (i = 0; i < AS_LIST(a)->count; i++) {
            list_append_object(res, AS_LIST(a)->items[i]);
        }
        for (i = 0; i < AS_LIST(b)->count; i++) {
            list_append_object(res, AS_LIST(b)->items[i]);
        }
    } else {
        DIE("Cannot append object of type %s to a list\n",
                b->type->type_name);
    }

    return res;
}

/**
 * Concatenates two LuciMapObjs
 *
 * @param a first LuciMapObj
 * @param b second LuciMapObj
 * @returns concatenated LuciMapObj
 */
static LuciObject* LuciMap_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = NULL;

    if (ISTYPE(b, obj_map_t)) {
        res = LuciMap_new();
        int i;
        LuciObject *key = NULL, *val = NULL;
        for (i = 0; i < AS_MAP(a)->size; i++) {
            key = AS_MAP(a)->keys[i];
            if (key) {
                map_set(res, key, AS_MAP(a)->vals[i]);
            }
        }
        for (i = 0; i < AS_MAP(b)->size; i++) {
            key = AS_MAP(b)->keys[i];
            if (key) {
                map_set(res, key, AS_MAP(b)->vals[i]);
            }
        }
    } else {
        DIE("Cannot append object of type %s to a map\n",
                b->type->type_name);
    }

    return res;
}


/**
 * Prints a LuciIntObj to stdout
 *
 * @param in LuciIntObj to print
 */
static void LuciInt_print(LuciObject *in)
{
    printf("%ld", AS_INT(in)->i);
}

/**
 * Prints a LuciFloatObj to stdout
 *
 * @param in LuciFloatObj to print
 */
static void LuciFloat_print(LuciObject *in)
{
    printf("%f", AS_FLOAT(in)->f);
}

/**
 * Prints a LuciStringObj to stdout
 *
 * @param in LuciStringObj to print
 */
static void LuciString_print(LuciObject *in)
{
    printf("%s", AS_STRING(in)->s);
}

/**
 * Prints a LuciListObj to stdout
 *
 * @param in LuciListObj to print
 */
static void LuciList_print(LuciObject *in)
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

/**
 * Prints a LuciMapObj to stdout
 *
 * @param in LuciMapObj to print
 */
static void LuciMap_print(LuciObject *in)
{
    int i;
    printf("{");
    for (i = 0; i < AS_MAP(in)->size; i++) {
        if (AS_MAP(in)->keys[i]) {
            LuciObject *key = AS_MAP(in)->keys[i];
            LuciObject *val = AS_MAP(in)->vals[i];
            printf("\"");
            key->type->print(key);
            printf("\":");
            val->type->print(val);
            printf(", ");
        }
    }
    printf("}");
}

/**
 * Prints a representation of a LuciFileObj to stdout
 *
 * @param in LuciFileObj to print
 */
static void LuciFile_print(LuciObject *in)
{
    switch (AS_FILE(in)->mode) {
        case f_read_m:
            printf("<file 'r'>");
            break;
        case f_write_m:
            printf("<file 'w'>");
            break;
        case f_append_m:
            printf("<file 'a'>");
            break;
        default:
            break;
    }
}

/**
 * Prints a representation of a LuciFunction to stdout
 *
 * @param in LuciFunctionObj to print
 */
static void LuciFunction_print(LuciObject *in)
{
    printf("<function>");
}

/**
 * Prints a representation of a LuciLibFunc to stdout
 *
 * @param in LuciLibFunc to print
 */
static void LuciLibFunc_print(LuciObject *in)
{
    printf("<libfunction>");
}

/**
 * Unary placeholder no-op type member
 *
 * @param a unused
 */
void unary_noop(LuciObject *a) {}

/**
 * Binary placeholder no-op type member
 *
 * @param a unused
 * @param b unused
 */
void binary_noop(LuciObject *a, LuciObject *b) {}

/**
 * Ternary placeholder no-op type member
 *
 * @param a unused
 * @param b unused
 * @param c unused
 */
void ternary_noop(LuciObject *a, LuciObject *b, LuciObject *c) {}


/**
 * Unary placeholder type member
 *
 * @param a unused
 * @returns NULL
 */
static LuciObject* unary_null(LuciObject *a)
{
    return NULL;
}

/**
 * Binary placeholder type member
 *
 * @param a unused
 * @param b unused
 * @returns NULL
 */
static LuciObject* binary_null(LuciObject *a, LuciObject *b)
{
    return NULL;
}

/**
 * Ternary placeholder type member
 *
 * @param a unused
 * @param b unused
 * @param c unused
 * @returns NULL
 */
static LuciObject* ternary_null(LuciObject *a, LuciObject *b, LuciObject *c)
{
    return NULL;
}


