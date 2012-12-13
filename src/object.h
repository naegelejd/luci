/*
 * See Copyright Notice in luci.h
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <stdio.h>  /* for FILE */
#include <stdint.h>


/* initial allocated size of a new List */
#define INIT_LIST_SIZE 32

/* Types of LuciObjects */
typedef enum {
    obj_int_t,
    obj_float_t,
    obj_str_t,
    obj_file_t,
    obj_list_t,
    obj_iterator_t,
    obj_func_t,
    obj_libfunc_t
} LuciObjType;

/* Object Types */
typedef long LuciIntObj;

typedef double LuciFloatObj;

typedef struct _LuciString {
    long len;
    char * s;
} LuciStringObj;

typedef struct _LuciFile {
    enum { f_read_m, f_write_m, f_append_m } mode;
    long size; /* in bytes */
    FILE * ptr;
} LuciFileObj;

typedef struct _LuciList {
    unsigned int count;	/* current number of items in list */
    unsigned int size;	/* current count of allocated items */
    struct _LuciObject **items;
} LuciListObj;

typedef struct _LuciMap {
    unsigned int count;	/* current number of key/value pairs */
    unsigned int size;	/* current count of allocated pairs*/
    struct _LuciObject **keys;
    struct _LuciObject **values;
} LuciMapObj;

typedef struct _LuciIterator {
    unsigned int idx;
    unsigned int step;
    struct _LuciObject *list;
} LuciIteratorObj;

typedef struct _LuciFunction {
    void *frame;
    void (*deleter)(void *);
} LuciFunctionObj;

/* Library function */
typedef struct _LuciObject * (*LuciLibFuncObj)(struct _LuciObject **, unsigned int);

typedef union _LuciImpl {
    LuciIntObj           i;
    LuciFloatObj         f;
    LuciStringObj        string;
    LuciFileObj          file;
    LuciListObj          list;
    LuciIteratorObj      iterator;
    LuciFunctionObj      func;
    LuciLibFuncObj       libfunc;
} LuciObjImpl;

/* Generic Object which allows for dynamic typing */
typedef struct _LuciObject
{
    LuciObjType type;
    int refcount;
    LuciObjImpl value;
} LuciObject;

/* creates and initializes a new LuciObject */
LuciObject *create_object(int type);

/* increments the object's refcount and returns it */
LuciObject *incref(LuciObject* orig);
/* decrements the object's refcount and returns it
 * also potentially destroys object (refcount <= 0) */
LuciObject *incref(LuciObject* orig);
/* duplicates a LuciObject, creating a new one */
LuciObject *copy_object(LuciObject* orig);
/* destroys an object */
void destroy(LuciObject *trash);

int list_append_object(LuciObject *list, LuciObject *item);
LuciObject *list_get_object(LuciObject *list, int index);
LuciObject *list_set_object(LuciObject *list, LuciObject *item, int index);
LuciObject *iterator_next_object(LuciObject *iterator);
#endif
