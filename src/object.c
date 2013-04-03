/*
 * See Copyright Notice in luci.h
 */

/**
 * @file object.c
 */

#include "luci.h"
#include "gc.h"
#include "object.h"


static LuciObject* LuciNil_copy(LuciObject *);
static LuciObject* LuciNil_asbool(LuciObject *);
static void LuciNil_print(LuciObject *);

static LuciObject* LuciIterator_copy(LuciObject *);
static LuciObject* LuciIterator_asbool(LuciObject *);

static LuciObject* LuciFile_asbool(LuciObject *);
static void LuciFile_print(LuciObject *);

static LuciObject* LuciFunction_copy(LuciObject *);
static LuciObject* LuciFunction_asbool(LuciObject *);
static void LuciFunction_print(LuciObject *in);

static LuciObject* LuciLibFunc_copy(LuciObject *);
static LuciObject* LuciLibFunc_asbool(LuciObject *);
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
LuciObject *LuciLibFunc_new(LuciCFunc fptr, char *help, int min_args)
{
    LuciLibFuncObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_libfunc_t);
    o->func = fptr;
    o->help = help;
    o->min_args = min_args;
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
    LuciLibFuncObj *tmp = (LuciLibFuncObj *)orig;
    return LuciLibFunc_new(tmp->func, tmp->help, tmp->min_args);
}


static LuciObject* LuciNil_asbool(LuciObject *o)
{
    return LuciInt_new(false);
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
 * Prints LuciNilObj to stdout
 *
 * @param in LuciNilObj
 */
static void LuciNil_print(LuciObject *in)
{
    printf("%s", "nil");
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
LuciObject* unary_nil(LuciObject *a)
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
LuciObject* binary_nil(LuciObject *a, LuciObject *b)
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
LuciObject* ternary_nil(LuciObject *a, LuciObject *b, LuciObject *c)
{
    return LuciNilObj;
}
