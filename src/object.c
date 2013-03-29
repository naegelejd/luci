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


/**
 * Makes a negative container index positive based on the container's length.
 *
 * e.g. if the index is -5 and the length is 5, the index
 * should be 2 because 7 - 5 - 2.
 * likewise, if the index is -14 and the length is 5, the index
 * should be 1.
 *
 * @param idx index to make positive
 * @param len length of container being indexed
 */
#define MAKE_INDEX_POS(idx, len) while ((idx) < 0) { (idx) = (len) - (idx); }


static LuciObject* LuciNil_copy(LuciObject *);
static LuciObject* LuciInt_copy(LuciObject *);
static LuciObject* LuciFloat_copy(LuciObject *);
static LuciObject* LuciIterator_copy(LuciObject *);
static LuciObject* LuciFunction_copy(LuciObject *);
static LuciObject* LuciLibFunc_copy(LuciObject *);

static LuciObject* LuciInt_repr(LuciObject *);
static LuciObject* LuciFloat_repr(LuciObject *);

static LuciObject* LuciNil_asbool(LuciObject *);
static LuciObject* LuciInt_asbool(LuciObject *);
static LuciObject* LuciFloat_asbool(LuciObject *);
static LuciObject* LuciFile_asbool(LuciObject *);
static LuciObject* LuciIterator_asbool(LuciObject *);
static LuciObject* LuciFunction_asbool(LuciObject *);
static LuciObject* LuciLibFunc_asbool(LuciObject *);

static LuciObject* LuciString_copy(LuciObject *);
static LuciObject* LuciString_repr(LuciObject *);
static LuciObject* LuciString_asbool(LuciObject *);
static LuciObject* LuciString_len(LuciObject *);
static LuciObject* LuciString_add(LuciObject *, LuciObject *);
static LuciObject* LuciString_mul(LuciObject *, LuciObject *);
static LuciObject* LuciString_eq(LuciObject *, LuciObject *);
static LuciObject* LuciString_contains(LuciObject *m, LuciObject *o);
static LuciObject* LuciString_cget(LuciObject *, LuciObject *);
static LuciObject* LuciString_cput(LuciObject *, LuciObject *, LuciObject *);
static LuciObject* LuciString_next(LuciObject *, LuciObject *);

static LuciObject* LuciList_copy(LuciObject *);
static LuciObject* LuciList_len(LuciObject *);
static LuciObject* LuciList_asbool(LuciObject *);
static LuciObject* LuciList_add(LuciObject *, LuciObject *);
static LuciObject* LuciList_eq(LuciObject *, LuciObject *);
static LuciObject* LuciList_contains(LuciObject *m, LuciObject *o);
static LuciObject* LuciList_cget(LuciObject *, LuciObject *);
static LuciObject* LuciList_cput(LuciObject *, LuciObject *, LuciObject *);
static LuciObject* LuciList_next(LuciObject *, LuciObject *);

static LuciObject* LuciMap_copy(LuciObject *);
static LuciObject* LuciMap_asbool(LuciObject *);
static LuciObject* LuciMap_len(LuciObject *);
static LuciObject* LuciMap_add(LuciObject *, LuciObject *);
static LuciObject* LuciMap_eq(LuciObject *, LuciObject *);
static LuciObject* LuciMap_contains(LuciObject *m, LuciObject *o);
static LuciObject* LuciMap_next(LuciObject *, LuciObject *);

static LuciObject* LuciInt_add(LuciObject *, LuciObject *);
static LuciObject* LuciInt_sub(LuciObject *, LuciObject *);
static LuciObject* LuciInt_mul(LuciObject *, LuciObject *);
static LuciObject* LuciInt_div(LuciObject *, LuciObject *);
static LuciObject* LuciInt_mod(LuciObject *, LuciObject *);
static LuciObject* LuciInt_pow(LuciObject *, LuciObject *);
static LuciObject* LuciInt_eq(LuciObject *, LuciObject *);
static LuciObject* LuciInt_neq(LuciObject *, LuciObject *);
static LuciObject* LuciInt_lt(LuciObject *, LuciObject *);
static LuciObject* LuciInt_gt(LuciObject *, LuciObject *);
static LuciObject* LuciInt_lte(LuciObject *, LuciObject *);
static LuciObject* LuciInt_gte(LuciObject *, LuciObject *);
static LuciObject* LuciInt_lgor(LuciObject *, LuciObject *);
static LuciObject* LuciInt_lgand(LuciObject *, LuciObject *);
static LuciObject* LuciInt_bwxor(LuciObject *, LuciObject *);
static LuciObject* LuciInt_bwor(LuciObject *, LuciObject *);
static LuciObject* LuciInt_bwand(LuciObject *, LuciObject *);

static LuciObject* LuciInt_neg(LuciObject *);
static LuciObject* LuciInt_lgnot(LuciObject *);
static LuciObject* LuciInt_bwnot(LuciObject *);

static LuciObject* LuciFloat_add(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_sub(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_mul(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_div(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_mod(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_pow(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_eq(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_neq(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_lt(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_gt(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_lte(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_gte(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_lgor(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_lgand(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_bwxor(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_bwor(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_bwand(LuciObject *, LuciObject *);

static LuciObject* LuciFloat_neg(LuciObject *);
static LuciObject* LuciFloat_lgnot(LuciObject *);
static LuciObject* LuciFloat_bwnot(LuciObject *);


static void LuciNil_print(LuciObject *);
static void LuciInt_print(LuciObject *);
static void LuciFloat_print(LuciObject *);
static void LuciString_print(LuciObject *);
static void LuciFile_print(LuciObject *);
static void LuciList_print(LuciObject *);
static void LuciMap_print(LuciObject *);
static void LuciFile_print(LuciObject *);
static void LuciFunction_print(LuciObject *in);
static void LuciLibFunc_print(LuciObject *in);


/** Type member table for LuciNilObj */
LuciObjectType obj_nil_t = {
    "nil",
    LuciNil_copy,
    unary_nil,
    LuciNil_asbool,
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

    LuciNil_print
};

/** Type member table for LuciIntObj */
LuciObjectType obj_int_t = {
    "int",
    LuciInt_copy,
    LuciInt_repr,
    LuciInt_asbool,
    unary_nil,
    LuciInt_neg,
    LuciInt_lgnot,
    LuciInt_bwnot,

    LuciInt_add,
    LuciInt_sub,
    LuciInt_mul,
    LuciInt_div,
    LuciInt_mod,
    LuciInt_pow,
    LuciInt_eq,
    LuciInt_neq,
    LuciInt_lt,
    LuciInt_gt,
    LuciInt_lte,
    LuciInt_gte,
    LuciInt_lgor,
    LuciInt_lgand,
    LuciInt_bwxor,
    LuciInt_bwor,
    LuciInt_bwand,

    binary_nil,
    binary_nil,
    binary_nil,

    ternary_nil,

    LuciInt_print
};

/** Type member table for LuciFloatObj */
LuciObjectType obj_float_t = {
    "float",
    LuciFloat_copy,
    LuciFloat_repr,
    LuciFloat_asbool,
    unary_nil,
    LuciFloat_neg,
    LuciFloat_lgnot,
    LuciFloat_bwnot,

    LuciFloat_add,
    LuciFloat_sub,
    LuciFloat_mul,
    LuciFloat_div,
    LuciFloat_mod,
    LuciFloat_pow,
    LuciFloat_eq,
    LuciFloat_neq,
    LuciFloat_lt,
    LuciFloat_gt,
    LuciFloat_lte,
    LuciFloat_gte,
    LuciFloat_lgor,
    LuciFloat_lgand,
    LuciFloat_bwxor,
    LuciFloat_bwor,
    LuciFloat_bwand,

    binary_nil,
    binary_nil,
    binary_nil,

    ternary_nil,

    LuciFloat_print
};

/** Type member table for LuciStringObj */
LuciObjectType obj_string_t = {
    "string",
    LuciString_copy,
    LuciString_repr,
    LuciString_asbool,
    LuciString_len,
    unary_nil,
    unary_nil,
    unary_nil,

    LuciString_add,
    binary_nil,
    LuciString_mul,
    binary_nil,
    binary_nil,
    binary_nil,
    LuciString_eq,
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

    LuciString_contains,
    LuciString_next,
    LuciString_cget,

    LuciString_cput,

    LuciString_print
};

/** Type member table for LuciListObj */
LuciObjectType obj_list_t = {
    "list",
    LuciList_copy,
    unary_nil,
    LuciList_asbool,
    LuciList_len,
    unary_nil,
    unary_nil,
    unary_nil,

    LuciList_add,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    LuciList_eq,
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

    LuciList_contains,
    LuciList_next,
    LuciList_cget,

    LuciList_cput,

    LuciList_print
};

/** Type member table for LuciMapObj */
LuciObjectType obj_map_t = {
    "map",
    LuciMap_copy,
    unary_nil,
    LuciMap_asbool,
    LuciMap_len,
    unary_nil,
    unary_nil,
    unary_nil,

    LuciMap_add,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    binary_nil,
    LuciMap_eq,
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

    LuciMap_contains,
    LuciMap_next,
    LuciMap_cget,

    LuciMap_cput,

    LuciMap_print
};

/** Type member table for LuciFileObj */
LuciObjectType obj_file_t = {
    "file",
    unary_nil,
    unary_nil,
    LuciFile_asbool,
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

    LuciFile_print
};

/** Type member table for LuciIteratorObj */
LuciObjectType obj_iterator_t = {
    "iterator",
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

    unary_void
};

/** Type member table for LuciFunctionObj */
LuciObjectType obj_func_t = {
    "function",
    LuciFunction_copy,
    unary_nil,
    LuciFunction_asbool,
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

    LuciFunction_print
};

/** Type member table for LuciLibFuncObj */
LuciObjectType obj_libfunc_t = {
    "libfunction",
    LuciLibFunc_copy,
    unary_nil,
    LuciLibFunc_asbool,
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

    LuciLibFunc_print
};


/** Definition of LuciNilObj */
LuciObject LuciNilInstance = {
    &obj_nil_t
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
LuciObject *LuciIterator_new(LuciObject *container, int step)
{
    LuciIteratorObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_iterator_t);
    o->idx = LuciInt_new(0);
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
    if (!list || (!ISTYPE(list, obj_list_t))) {
	DIE("%s", "Can't append item to non-list object\n");
    }

    LuciListObj *listobj = (LuciListObj *)list;

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
LuciObject *list_get_object(LuciObject *list, long index)
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
LuciObject *list_set_object(LuciObject *list, LuciObject *item, long index)
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
 * Returns the 'next' char in the string
 *
 * @param str LuciStringObj
 * @param idx index
 * @returns char at index idx or NULL if out of bounds
 */
LuciObject *LuciString_next(LuciObject *str, LuciObject *idx)
{
    if (!ISTYPE(idx, obj_int_t)) {
        DIE("%s\n", "Argument to LuciString_next must be LuciIntObj");
    }

    if (AS_INT(idx)->i >= AS_STRING(str)->len) {
        return NULL;
    }

    char *s = alloc(2 * sizeof(char));
    s[0] = AS_STRING(str)->s[AS_INT(idx)->i];
    s[1] = '\0';
    return LuciString_new(s);
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
 * Returns the 'next' object in the map
 *
 * @param m LuciMapObj
 * @param idx index
 * @returns copy of object at index idx or NULL if out of bounds
 */
LuciObject *LuciMap_next(LuciObject *m, LuciObject *idx)
{
    if (!ISTYPE(idx, obj_int_t)) {
        DIE("%s\n", "Argument to LuciMap_next must be LuciIntObj");
    }

    if (AS_INT(idx)->i >= AS_MAP(m)->count) {
        return NULL;
    }

    /* loop through the map keys, counting keys until we reach idx */
    int i = 0, count = -1;
    for (i = 0; i < AS_MAP(m)->size; i++) {
        if (AS_MAP(m)->keys[i]) {
            count++;
        }
        if (count == AS_INT(idx)->i) {
            return AS_MAP(m)->keys[i];
        }
    }
    return NULL;
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

    long idx = AS_INT(iter->idx)->i;
    if (ISTYPE(container, obj_string_t)) {
        LuciStringObj *str = (LuciStringObj *)container;
        if (idx >= str->len) {
            return NULL;
        } else {
            char *s = alloc(2 * sizeof(char));
            s[0] = str->s[idx];
            s[1] = '\0';

            LuciObject *item = LuciString_new(s);

            AS_INT(iter->idx)->i += iter->step;

            return item;
        }
    } else if (ISTYPE(container, obj_list_t)) {
        LuciListObj *list = (LuciListObj *)container;

        if (idx >= list->count) {
            return NULL;
        } else {
            /* get the item from the list */
            LuciObject *item = list->items[idx];
            /* update the iterator's index */
            AS_INT(iter->idx)->i += iter->step;
            /* return a copy of the item */
            return item->type->copy(item);
        }
    } else if (ISTYPE(container, obj_map_t)) {
        LuciMapObj *map = (LuciMapObj *)container;

        if (idx >= map->size) {
            return NULL;
        }

        if (idx == 0) {
            /* be sure we're at a valid 'index' in the map's hash table */
            while ((idx < map->size) && (map->keys[idx] == NULL)) {
                idx += 1;
            }
        }
        /* save this valid index to return the corresponding key */
        long this_idx = idx;

        do {
            /* increment the index for the next call */
            idx += iter->step;
            /* this will leave the idx at either a valid key
             * or the last index in the map's key array */
        } while ((idx < map->size) && (map->keys[idx] == NULL));

        /* grab the key at the saved index */
        LuciObject *key = map->keys[this_idx];

        /* update the iterator's index */
        AS_INT(iter->idx)->i = idx;

        /* return a copy of the key */
        return key->type->copy(key);
    } else {
        DIE("%s\n", "Cannot iterate over a non-container type");
    }

    return NULL;
}

/**
 * Copies a LuciNilObj, which is just the same instance
 *
 * @param orig LuciNilObj
 * @returns same LuciNilObj
 */
static LuciObject* LuciNil_copy(LuciObject *orig)
{
    return orig;
}


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
            LuciMap_cput(copy, key->type->copy(key), val->type->copy(val));
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

static LuciObject* LuciNil_asbool(LuciObject *o)
{
    return LuciInt_new(false);
}

static LuciObject* LuciInt_asbool(LuciObject *o)
{
    return LuciInt_new(AS_INT(o)->i > 0L);
}

static LuciObject* LuciFloat_asbool(LuciObject *o)
{
    return LuciInt_new(AS_FLOAT(o)->f > 0.0L);
}

static LuciObject* LuciString_asbool(LuciObject *o)
{
    return LuciInt_new(AS_STRING(o)->len > 0);
}

static LuciObject* LuciList_asbool(LuciObject *o)
{
    return LuciInt_new(AS_LIST(o)->count > 0);
}

static LuciObject* LuciMap_asbool(LuciObject *o)
{
    return LuciInt_new(AS_MAP(o)->count > 0);
}

static LuciObject* LuciFile_asbool(LuciObject *o)
{
    LuciObject *res = LuciNilObj;

    if (AS_FILE(o)->ptr) {
        res = LuciInt_new(true);
    } else {
        res = LuciInt_new(false);
    }
    return res;
}

static LuciObject* LuciIterator_asbool(LuciObject *o)
{
    LuciObject *res = LuciNilObj;

    LuciObject *container = AS_ITERATOR(o)->container;
    unsigned int len;
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

static LuciObject* LuciFunction_asbool(LuciObject *o)
{
    return LuciInt_new(true);
}

static LuciObject* LuciLibFunc_asbool(LuciObject *o)
{
    return LuciInt_new(true);
}


/**
 * Returns the length of a LuciStringObj
 *
 * @param o LuciStringObj
 * @returns length of o
 */
static LuciObject* LuciString_len(LuciObject *o)
{
    return LuciInt_new(AS_STRING(o)->len);
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
    if (ISTYPE(b, obj_string_t)) {
        char *s = alloc(AS_STRING(a)->len + AS_STRING(b)-> len + 1);
        strncpy(s, AS_STRING(a)->s, AS_STRING(a)->len);
        strncat(s, AS_STRING(b)->s, AS_STRING(b)->len);
        return LuciString_new(s);
    } else {
        DIE("Cannot append object of type %s to a string\n",
                b->type->type_name);
    }

    return LuciNilObj;
}

/**
 * Concatenates a LuciStringObj b times
 *
 * @param a LuciStringObj to multiply
 * @param b integer multiplier
 * @returns concatenated LuciStringObj
 */
static LuciObject* LuciString_mul(LuciObject *a, LuciObject *b)
{
    if (ISTYPE(b, obj_int_t)) {
        char *s = alloc(AS_STRING(a)->len * AS_INT(b)->i + 1);
        *s = '\0';

        int i;
        for (i = 0; i < AS_INT(b)->i; i++) {
            strncat(s, AS_STRING(a)->s, AS_STRING(a)->len);
        }
        return LuciString_new(s);
    } else {
        DIE("Cannot multiply a string by an object of type %s\n",
                b->type->type_name);
    }

    return LuciNilObj;
}

/**
 * Determines if two LuciStringObjs are equal
 *
 * @param a LuciStringObj
 * @param b LuciStringObj
 * @returns 1 if equal, 0 otherwise
 */
static LuciObject* LuciString_eq(LuciObject *a, LuciObject *b)
{
    if(ISTYPE(b, obj_string_t)) {
        if (AS_STRING(a)->len != AS_STRING(b)->len) {
            return LuciInt_new(false);
        }
        if ((strncmp(AS_STRING(a)->s, AS_STRING(b)->s, AS_STRING(a)->len)) == 0) {
            return LuciInt_new(true);
        } else {
            return LuciInt_new(false);
        }
    } else {
        DIE("Cannot compare a string to an object of type %s\n",
                b->type->type_name);
    }
    return LuciNilObj;
}

/**
 * Determines whether a LuciStringObj contains an object
 *
 * @param str LuciStringObj
 * @param o object
 * @returns 1 if str contains o, 0 otherwise
 */
static LuciObject *LuciString_contains(LuciObject *str, LuciObject *o)
{
    if (!ISTYPE(o, obj_string_t)) {
        DIE("A string can only contain a string, not a %s\n",
                o->type->type_name);
    }
    if ((strstr(AS_STRING(str)->s, AS_STRING(o)->s)) != NULL) {
        return LuciInt_new(true);
    } else {
        return LuciInt_new(false);
    }
}

/**
 * Gets the character at index b in LuciStringObj a
 *
 * @param a LuciStringObj
 * @param b index in a
 * @returns character at index b
 */
static LuciObject* LuciString_cget(LuciObject *a, LuciObject *b)
{
    if (ISTYPE(b, obj_int_t)) {
        long idx = AS_INT(b)->i;

        MAKE_INDEX_POS(idx, AS_STRING(a)->len);

        if (idx >= AS_STRING(a)->len) {
            DIE("%s\n", "String subscript out of bounds");
        }

        char *s = alloc(2 * sizeof(char));
        s[0] = AS_STRING(a)->s[idx];
        s[1] = '\0';
        return LuciString_new(s);
    } else {
        DIE("Cannot subscript a string with an object of type %s\n",
                b->type->type_name);
    }
    return LuciNilObj;
}

/**
 * Sets the character at index b in LuciStringObj a
 *
 * @param a LuciStringObj
 * @param b index in a
 * @returns former character at index b
 */
static LuciObject* LuciString_cput(LuciObject *a, LuciObject *b, LuciObject *c)
{
    if (ISTYPE(b, obj_int_t)) {
        if (ISTYPE(c, obj_string_t)) {
            long idx = AS_INT(b)->i;
            MAKE_INDEX_POS(idx, AS_STRING(a)->len);
            if (idx >= AS_STRING(a)->len) {
                DIE("%s\n", "String subscript out of bounds");
            }
            char *s = alloc(2 * sizeof(char));
            s[0] = AS_STRING(a)->s[idx];
            s[1] = '\0';

            /* just put one char for now */
            AS_STRING(a)->s[idx] = AS_STRING(c)->s[0];

            /* return the former char */
            return LuciString_new(s);
        } else {
            DIE("Cannot put an object of type %s into a string\n", 
                    c->type->type_name);
        }
    } else {
        DIE("Cannot subscript a string with an object of type %s\n",
                b->type->type_name);
    }
    return LuciNilObj;
}

/**
 * Returns the length of a LuciListObj
 *
 * @param o LuciListObj
 * @returns length of o
 */
static LuciObject* LuciList_len(LuciObject *o)
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
static LuciObject* LuciList_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

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
 * Determines if two LuciListObjs are equal
 *
 * @param a LuciListObj
 * @param b LuciListObj
 * @returns 1 if equal, 0 otherwise
 */
static LuciObject* LuciList_eq(LuciObject *a, LuciObject *b)
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
static LuciObject *LuciList_contains(LuciObject *l, LuciObject *o)
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
 * Gets the object at index b in LuciListObj a
 *
 * @param a LuciListObj
 * @param b index in a
 * @returns object at index b
 */
static LuciObject* LuciList_cget(LuciObject *a, LuciObject *b)
{
    if (ISTYPE(b, obj_int_t)) {
        return list_get_object(a, AS_INT(b)->i);
    } else {
        DIE("Cannot subscript a list with an object of type %s\n",
                b->type->type_name);
    }

    return LuciNilObj;
}

static LuciObject* LuciList_cput(LuciObject *a, LuciObject *b, LuciObject *c)
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
 * Returns the length of a LuciMapObj
 *
 * @param o LuciMapObj
 * @returns length of o
 */
static LuciObject* LuciMap_len(LuciObject *o)
{
    return LuciInt_new(AS_MAP(o)->count);
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
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_map_t)) {
        res = LuciMap_new();
        int i;
        LuciObject *key = NULL, *val = NULL;
        for (i = 0; i < AS_MAP(a)->size; i++) {
            key = AS_MAP(a)->keys[i];
            if (key) {
                LuciMap_cput(res, key, AS_MAP(a)->vals[i]);
            }
        }
        for (i = 0; i < AS_MAP(b)->size; i++) {
            key = AS_MAP(b)->keys[i];
            if (key) {
                LuciMap_cput(res, key, AS_MAP(b)->vals[i]);
            }
        }
    } else {
        DIE("Cannot append object of type %s to a map\n",
                b->type->type_name);
    }

    return res;
}

/**
 * Determines if two LuciMapObjs are equal
 *
 * @param a LuciMapObj
 * @param b LuciMapObj
 * @returns 1 if equal, 0 otherwise
 */
static LuciObject* LuciMap_eq(LuciObject *a, LuciObject *b)
{
    if(ISTYPE(b, obj_map_t)) {
        if (AS_MAP(a)->count != AS_MAP(b)->count) {
            return LuciInt_new(false);
        }
        int i;
        for (i = 0; i < AS_MAP(a)->size; i++) {
            LuciObject *key = AS_MAP(a)->keys[i];
            if (key) {
                LuciObject *val1 = AS_MAP(b)->vals[i];
                /* TODO: if the key isn't in the second map,
                 * this will DIE and kill luci */
                LuciObject *val2 = LuciMap_cget(b, key);
                LuciObject *eq = val1->type->eq(val1, val2);
                /* if the objects in the lists at index i aren't equal,
                 * return false */
                if (!AS_INT(eq)->i) {
                    return LuciInt_new(false);
                }
            }
        }
        /* all key-value pairs are in both maps */
        return LuciInt_new(true);
    } else {
        DIE("Cannot compare a map to an object of type %s\n",
                b->type->type_name);
    }
    return LuciNilObj;
}

/**
 * Determines whether a LuciMapObj contains a key
 *
 * @param m LuciMapObj
 * @param o object
 * @returns 1 if str contains k, 0 otherwise
 */
static LuciObject *LuciMap_contains(LuciObject *m, LuciObject *o)
{
    int i;
    for (i = 0; i < AS_MAP(m)->size; i++) {
        LuciObject *key = AS_MAP(m)->keys[i];
        if (key) {
            LuciObject *eq = o->type->eq(o, key);
            if (AS_INT(eq)->i) {
                return LuciInt_new(true);
            }
        }
    }
    return LuciInt_new(false);
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
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i + AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i + AS_FLOAT(b)->f);
    } else {
        DIE("Cannot add object of type %s to an int\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_sub(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i - AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i - AS_FLOAT(b)->f);
    } else {
        DIE("Cannot subtract an object of type %s from an int\n", b->type->type_name);
    }
    return res;
}


static LuciObject* LuciInt_mul(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i * AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i * AS_FLOAT(b)->f);
    } else {
        DIE("Cannot multiply an object of type %s and an int\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_div(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        if (AS_INT(b)->i != 0) {
            res = LuciInt_new(AS_INT(a)->i / AS_INT(b)->i);
        } else {
            DIE("%s\n", "Divide by zero");
        }
    } else if (ISTYPE(b, obj_float_t)) {
        if (AS_FLOAT(b)->f != 0.0L) {
            res = LuciFloat_new(AS_INT(a)->i / AS_FLOAT(b)->f);
        } else {
            DIE("%s\n", "Divide by zero");
        }
    } else {
        DIE("Cannot divide an int by an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_mod(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        if (AS_INT(b)->i != 0) {
            res = LuciInt_new(AS_INT(a)->i % AS_INT(b)->i);
        } else {
            DIE("%s\n", "Modulus divide by zero");
        }
    } else {
        DIE("Cannot compute int modulus using an object of type %s\n",
                b->type->type_name);
    }

    return res;
}

static LuciObject* LuciInt_pow(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(pow(AS_INT(a)->i, AS_INT(b)->i));
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(pow(AS_INT(a)->i, AS_FLOAT(b)->f));
    } else {
        DIE("Cannot compute the power of an int using an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_eq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i == AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i == AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_neq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i != AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i != AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_lt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i < AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i < AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is less than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_gt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i > AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i > AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is greater than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_lte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i <= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i <= AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is less than or equal to an "
                "object of type %s\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_gte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i >= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i >= AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is greater than or equal to an "
                "object of type %s\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_lgor(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = a->type->asbool(a);

    return LuciInt_new(AS_INT(a0)->i || AS_INT(b0)->i);
}

static LuciObject* LuciInt_lgand(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = a->type->asbool(a);

    return LuciInt_new(AS_INT(a0)->i && AS_INT(b0)->i);
}

static LuciObject* LuciInt_bwxor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i ^ AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i ^ (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise XOR of an int and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_bwor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i | AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i | (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise OR of an int and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_bwand(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i & AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i & (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise AND of an int and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciInt_lgnot(LuciObject *a)
{
    LuciObject *a0 = a->type->asbool(a);
    return LuciInt_new(!(AS_INT(a0)->i));
}

static LuciObject* LuciInt_bwnot(LuciObject *a)
{
    return LuciInt_new(~(AS_INT(a)->i));
}

static LuciObject *LuciInt_neg(LuciObject *a)
{
    return LuciInt_new(-(AS_INT(a)->i));
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
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f + AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f + AS_FLOAT(b)->f);
    } else {
        DIE("Cannot add object of type %s to a float\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_sub(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f - AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f - AS_FLOAT(b)->f);
    } else {
        DIE("Cannot subtract an object of type %s from a float\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_mul(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f * AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f * AS_FLOAT(b)->f);
    } else {
        DIE("Cannot multiply an object of type %s and a float\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_div(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        if (AS_INT(b)->i != 0) {
            res = LuciFloat_new(AS_FLOAT(a)->f / AS_INT(b)->i);
        } else {
            DIE("%s\n", "Divide by zero");
        }
    } else if (ISTYPE(b, obj_float_t)) {
        if (AS_FLOAT(b)->f != 0.0L) {
            res = LuciFloat_new(AS_FLOAT(a)->f / AS_FLOAT(b)->f);
        } else {
            DIE("%s\n", "Divide by zero");
        }
    } else {
        DIE("Cannot divide a float by an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_mod(LuciObject *a, LuciObject *b)
{
    DIE("%s\n", "Cannot compute float modulus");
}

static LuciObject* LuciFloat_pow(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(pow(AS_FLOAT(a)->f, AS_INT(b)->i));
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(pow(AS_FLOAT(a)->f, AS_FLOAT(b)->f));
    } else {
        DIE("Cannot compute the power of a float using an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_eq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f == AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f == AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_neq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f != AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f != AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_lt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f < AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f < AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is less than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_gt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f > AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f > AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is greater than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_lte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f <= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f <= AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is less than or equal "
                "to an object of type %s\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_gte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f >= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f >= AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is greater than or equal "
                "to an object of type %s\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_lgor(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = a->type->asbool(a);

    return LuciInt_new(AS_INT(a0)->i || AS_INT(b0)->i);
}

static LuciObject* LuciFloat_lgand(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = a->type->asbool(a);

    return LuciInt_new(AS_INT(a0)->i && AS_INT(b0)->i);
}

static LuciObject* LuciFloat_bwxor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f ^ AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f ^ (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise XOR of a float and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_bwor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f | AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f | (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise OR of a float and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_bwand(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f & AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f & (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise AND of a float and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_lgnot(LuciObject *a)
{
    LuciObject *a0 = a->type->asbool(a);
    return LuciInt_new(!(AS_INT(a0)->i));
}

static LuciObject* LuciFloat_bwnot(LuciObject *a)
{
    return LuciInt_new(~((long)AS_FLOAT(a)->f));
}

static LuciObject *LuciFloat_neg(LuciObject *a)
{
    return LuciFloat_new(-(AS_FLOAT(a)->f));
}


/**
 * Prints LuciNilObj to stdout
 *
 * @param in LuciNilObj
 */
static void LuciNil_print(LuciObject *in)
{
    printf("%s", "nil");
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
void unary_void(LuciObject *a) {}

/**
 * Binary placeholder no-op type member
 *
 * @param a unused
 * @param b unused
 */
void binary_void(LuciObject *a, LuciObject *b) {}

/**
 * Ternary placeholder no-op type member
 *
 * @param a unused
 * @param b unused
 * @param c unused
 */
void ternary_void(LuciObject *a, LuciObject *b, LuciObject *c) {}


/**
 * Unary placeholder type member
 *
 * @param a unused
 * @returns LuciNilObj
 */
static LuciObject* unary_nil(LuciObject *a)
{
    return LuciNilObj;
}

/**
 * Binary placeholder type member
 *
 * @param a unused
 * @param b unused
 * @returns LuciNilObj
 */
static LuciObject* binary_nil(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

/**
 * Ternary placeholder type member
 *
 * @param a unused
 * @param b unused
 * @param c unused
 * @returns LuciNilObj
 */
static LuciObject* ternary_nil(LuciObject *a, LuciObject *b, LuciObject *c)
{
    return LuciNilObj;
}


