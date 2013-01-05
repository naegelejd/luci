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

/* Generic Object which allows for dynamic typing */
typedef struct _LuciObject
{
    LuciObjType type;
    int refcount;
} LuciObject;


/* Object Types */
typedef struct _LuciIntObj
{
    LuciObject base;
    long i;
} LuciIntObj;

typedef struct _LuciFloatObj
{
    LuciObject base;
    double f;
} LuciFloatObj;

typedef struct _LuciString {
    LuciObject base;
    long len;
    char * s;
} LuciStringObj;

typedef struct _LuciFile {
    LuciObject base;
    enum { f_read_m, f_write_m, f_append_m } mode;
    long size; /* in bytes */
    FILE * ptr;
} LuciFileObj;

typedef struct _LuciList {
    LuciObject base;
    unsigned int count;	/* current number of items in list */
    unsigned int size;	/* current count of allocated items */
    LuciObject **items;
} LuciListObj;

typedef struct _LuciMap {
    LuciObject base;
    unsigned int count;	/* current number of key/value pairs */
    unsigned int size;	/* current count of allocated pairs*/
    LuciObject **keys;
    LuciObject **values;
} LuciMapObj;

typedef struct _LuciIterator {
    LuciObject base;
    unsigned int idx;
    unsigned int step;
    LuciObject *list;
} LuciIteratorObj;

typedef struct _LuciFunction {
    LuciObject base;
    void *frame;
} LuciFunctionObj;

/* Library function */
typedef struct _LuciLibFunc {
    LuciObject base;
    LuciObject * (*func)(LuciObject **, unsigned int);
} LuciLibFuncObj;


#define REFCOUNT(o) (((LuciObject *)(o))->refcount)
#define TYPEOF(o)   (((LuciObject *)(o))->type)

#define INCREF(o)   (((LuciObject *)(o))->refcount++)

#define TYPES_MATCH(left, right) ( TYPEOF(left) == TYPEOF(right) )

#define AS_INT(o)   ((LuciIntObj *)(o))
#define AS_FLOAT(o) ((LuciFloatObj *)(o))
#define AS_STRING(o) ((LuciStringObj *)(o))
#define AS_LIST(o) ((LuciListObj *)(o))
#define AS_FILE(o) ((LuciFileObj *)(o))


LuciObject *LuciInt_new(long l);
LuciObject *LuciFloat_new(double d);
LuciObject *LuciString_new(char *s);
LuciObject *LuciFile_new(FILE *fp, long size, int mode);
LuciObject *LuciList_new();
LuciObject *LuciIterator_new(LuciObject *list, unsigned int step);
LuciObject *LuciFunction_new(void *frame);
LuciObject *LuciLibFunc_new(LuciObject * (*func)(LuciObject **, unsigned int));

/* decrements the object's refcount and returns it
 * also potentially destroys object (refcount <= 0) */
LuciObject *decref(LuciObject* orig);

/* duplicates a LuciObject, creating a new one */
LuciObject *copy_object(LuciObject* orig);

/* destroys an object */
//void destroy(LuciObject *trash);

int list_append_object(LuciObject *list, LuciObject *item);
LuciObject *list_get_object(LuciObject *list, int index);
LuciObject *list_set_object(LuciObject *list, LuciObject *item, int index);
LuciObject *iterator_next_object(LuciObject *iterator);

#endif
