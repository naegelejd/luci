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

static LuciObject* LuciNil_copy(LuciObject *);
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

static LuciObject* LuciNil_asbool(LuciObject *);
static LuciObject* LuciInt_asbool(LuciObject *);
static LuciObject* LuciFloat_asbool(LuciObject *);
static LuciObject* LuciString_asbool(LuciObject *);
static LuciObject* LuciList_asbool(LuciObject *);
static LuciObject* LuciMap_asbool(LuciObject *);
static LuciObject* LuciFile_asbool(LuciObject *);
static LuciObject* LuciIterator_asbool(LuciObject *);
static LuciObject* LuciFunction_asbool(LuciObject *);
static LuciObject* LuciLibFunc_asbool(LuciObject *);

static LuciObject* LuciString_add(LuciObject *, LuciObject *);
static LuciObject* LuciList_add(LuciObject *, LuciObject *);
static LuciObject* LuciMap_add(LuciObject *, LuciObject *);

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
static LuciObject* LuciInt_lgnot(LuciObject *, LuciObject *);
static LuciObject* LuciInt_bxor(LuciObject *, LuciObject *);
static LuciObject* LuciInt_bor(LuciObject *, LuciObject *);
static LuciObject* LuciInt_band(LuciObject *, LuciObject *);
static LuciObject* LuciInt_bnot(LuciObject *, LuciObject *);

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
static LuciObject* LuciFloat_lgnot(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_bxor(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_bor(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_band(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_bnot(LuciObject *, LuciObject *);


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
    LuciNil_print
};

/** Type member table for LuciIntObj */
LuciObjectType obj_int_t = {
    "int",
    LuciInt_copy,
    LuciInt_repr,
    LuciInt_asbool,

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
    LuciInt_lgnot,
    LuciInt_bxor,
    LuciInt_bor,
    LuciInt_band,
    LuciInt_bnot,

    LuciInt_print
};

/** Type member table for LuciFloatObj */
LuciObjectType obj_float_t = {
    "float",
    LuciFloat_copy,
    LuciFloat_repr,
    LuciFloat_asbool,

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
    LuciFloat_lgnot,
    LuciFloat_bxor,
    LuciFloat_bor,
    LuciFloat_band,
    LuciFloat_bnot,

    LuciFloat_print
};

/** Type member table for LuciStringObj */
LuciObjectType obj_string_t = {
    "string",
    LuciString_copy,
    LuciString_repr,
    LuciString_asbool,

    LuciString_add,
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
    LuciString_print
};

/** Type member table for LuciListObj */
LuciObjectType obj_list_t = {
    "list",
    LuciList_copy,
    unary_nil,
    LuciList_asbool,

    LuciList_add,
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
    LuciList_print
};

/** Type member table for LuciMapObj */
LuciObjectType obj_map_t = {
    "map",
    LuciMap_copy,
    unary_nil,
    LuciMap_asbool,

    LuciMap_add,
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
    LuciMap_print
};

/** Type member table for LuciFileObj */
LuciObjectType obj_file_t = {
    "file",
    unary_nil,
    unary_nil,
    LuciFile_asbool,

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
    LuciFile_print
};

/** Type member table for LuciIteratorObj */
LuciObjectType obj_iterator_t = {
    "iterator",
    LuciIterator_copy,
    unary_nil,
    LuciIterator_asbool,

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
    unary_void
};

/** Type member table for LuciFunctionObj */
LuciObjectType obj_func_t = {
    "function",
    LuciFunction_copy,
    unary_nil,
    LuciFunction_asbool,

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
    LuciFunction_print
};

/** Type member table for LuciLibFuncObj */
LuciObjectType obj_libfunc_t = {
    "libfunction",
    LuciLibFunc_copy,
    unary_nil,
    LuciLibFunc_asbool,

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
LuciObject *list_get_object(LuciObject *list, int index)
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
LuciObject *list_set_object(LuciObject *list, LuciObject *item, int index)
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

    if (AS_ITERATOR(o)->idx < len) {
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
 * Concatenates two LuciStringObjs
 *
 * @param a first LuciStringObj
 * @param b second LuciStringObj
 * @returns concatenated LuciStringObj
 */
static LuciObject* LuciString_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

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
        res = LuciFloat_new(AS_INT(a)->i == AS_FLOAT(b)->f);
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
        res = LuciFloat_new(AS_INT(a)->i != AS_FLOAT(b)->f);
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
        res = LuciFloat_new(AS_INT(a)->i < AS_FLOAT(b)->f);
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
        res = LuciFloat_new(AS_INT(a)->i > AS_FLOAT(b)->f);
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
        res = LuciFloat_new(AS_INT(a)->i <= AS_FLOAT(b)->f);
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
        res = LuciFloat_new(AS_INT(a)->i >= AS_FLOAT(b)->f);
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

static LuciObject* LuciInt_lgnot(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    return LuciInt_new(!(AS_INT(a0)->i));
}

static LuciObject* LuciInt_bxor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i ^ AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i ^ (long)AS_FLOAT(b)->f);
    }
    return res;
}

static LuciObject* LuciInt_bor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i | AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i | (long)AS_FLOAT(b)->f);
    }
    return res;
}

static LuciObject* LuciInt_band(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i & AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i & (long)AS_FLOAT(b)->f);
    }
    return res;
}

static LuciObject* LuciInt_bnot(LuciObject *a, LuciObject *b)
{
    return LuciInt_new(~(AS_INT(a)->i));
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
    return LuciNilObj;
}

static LuciObject* LuciFloat_div(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_mod(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_pow(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_eq(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_neq(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_lt(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_gt(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_lte(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_gte(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_lgor(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_lgand(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_lgnot(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_bxor(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_bor(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_band(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
}

static LuciObject* LuciFloat_bnot(LuciObject *a, LuciObject *b)
{
    return LuciNilObj;
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


