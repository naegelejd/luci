/*
 * See Copyright Notice in luci.h
 */

/**
 * @file object.h
 *
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <stdio.h>  /* for FILE */
#include <stdint.h>


#define INIT_LIST_SIZE 32   /**< initial allocated size of a list */
#define INIT_MAP_SIZE 8     /**< initial allocated size of a map */


/** Types of LuciObjects */
typedef enum {
    obj_int_t,
    obj_float_t,
    obj_str_t,
    obj_file_t,
    obj_list_t,
    obj_map_t,
    obj_iterator_t,
    obj_func_t,
    obj_libfunc_t
} LuciObjType;

/** Generic Object which allows for dynamic typing */
typedef struct _LuciObject
{
    LuciObjType type;   /**< type ID - to be replaced by type pointer */
    int padding;        /**< to be deleted */
} LuciObject;


/** Integer object Type */
typedef struct _LuciIntObj
{
    LuciObject base;    /**< base implementation */
    long i;             /**< 'long' numerical value */
} LuciIntObj;

/** Floating-point object type */
typedef struct _LuciFloatObj
{
    LuciObject base;    /**< base implementation */
    double f;           /**< double-precision floating point value */
} LuciFloatObj;

/** String object type */
typedef struct _LuciString {
    LuciObject base;    /**< base implementation */
    long len;           /**< string length */
    char * s;           /**< pointer to C-string */
} LuciStringObj;

/** File open type */
typedef enum { f_read_m, f_write_m, f_append_m } file_mode;

/** File object type */
typedef struct _LuciFile {
    LuciObject base;    /**< base implementation */
    long size;          /**< file length in bytes */
    FILE * ptr;         /**< pointer to C-file pointer */
    file_mode mode;     /**< current mode file was opened in */
} LuciFileObj;

/** List object type */
typedef struct _LuciList {
    LuciObject base;    /**< base implementation */
    unsigned int count;	/**< current number of items in list */
    unsigned int size;	/**< current count of allocated items */
    LuciObject **items; /**< pointer to items array */
} LuciListObj;

/** Map object type */
typedef struct _LuciMap {
    LuciObject base;    /**< base implementation */
    unsigned int size_idx;  /**< identifier for current size of table */
    unsigned int collisions;    /**< number of hash collisions */
    unsigned int count;	/**< current number of key/value pairs */
    unsigned int size;	/**< current count of allocated pairs*/
    LuciObject **keys;  /**< array of pointers to keys */
    LuciObject **vals;  /**< array of pointers to values */
} LuciMapObj;

/** Iterator object type (internal) */
typedef struct _LuciIterator {
    LuciObject base;    /**< base implemenatation */
    unsigned int idx;   /**< current index */
    unsigned int step;  /**< amount to increment by */
    LuciObject *container;  /**< the container this iterator applies to */
} LuciIteratorObj;

/** User-defined function type */
typedef struct _LuciFunction {
    LuciObject base;    /**< base implementation */
    void *frame;        /**< pointer to Frame struct */
} LuciFunctionObj;

/** Library function type */
typedef struct _LuciLibFunc {
    LuciObject base;        /**< base implementation */
    LuciObject * (*func)(LuciObject **, unsigned int);  /**< function pointer */
} LuciLibFuncObj;

/** returns the type of object o */
#define TYPEOF(o)   (((LuciObject *)(o))->type)

/** returns 1 if two objects have the same type, 0 otherwise */
#define TYPES_MATCH(left, right) ( TYPEOF(left) == TYPEOF(right) )

/** casts LuciObject o to a LuciIntObj */
#define AS_INT(o)       ((LuciIntObj *)(o))
/** casts LuciObject o to a LuciFloatObj */
#define AS_FLOAT(o)     ((LuciFloatObj *)(o))
/** casts LuciObject o to a LuciStringObj */
#define AS_STRING(o)    ((LuciStringObj *)(o))
/** casts LuciObject o to a LuciListObj */
#define AS_LIST(o)      ((LuciListObj *)(o))
/** casts LuciObject o to a LuciMapObj */
#define AS_MAP(o)       ((LuciMapObj *)(o))
/** casts LuciObject o to a LuciFileObj */
#define AS_FILE(o)      ((LuciFileObj *)(o))
/** casts LuciObject o to a LuciMapObj */
#define AS_FUNCTION(o)  ((LuciFunctionObj *)(o))


LuciObject *LuciInt_new(long l);
LuciObject *LuciFloat_new(double d);
LuciObject *LuciString_new(char *s);
LuciObject *LuciFile_new(FILE *fp, long size, file_mode mode);
LuciObject *LuciList_new();
LuciObject *LuciIterator_new(LuciObject *list, unsigned int step);
LuciObject *LuciFunction_new(void *frame);
LuciObject *LuciLibFunc_new(LuciObject * (*func)(LuciObject **, unsigned int));

/** Used by print. Useful in debugging */
void print_object(LuciObject *);

/* Duplicates a LuciObject, allocating the new one */
LuciObject *copy_object(LuciObject* orig);

/* destroys an object */
//void destroy(LuciObject *trash);

unsigned int string_hash_0(LuciObject *s);
unsigned int string_hash_1(LuciObject *s);
unsigned int string_hash_2(LuciObject *s);

int list_append_object(LuciObject *list, LuciObject *item);
LuciObject *list_get_object(LuciObject *list, int index);
LuciObject *list_set_object(LuciObject *list, LuciObject *item, int index);
LuciObject *iterator_next_object(LuciObject *iterator);

#endif
