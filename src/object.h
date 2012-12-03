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
} LuciOType;

typedef enum {
    f_read_m,
    f_write_m,
    f_append_m
} LuciFileMode;

typedef union {
    int i;
    double f;
    char *s;
    struct {
        FILE *ptr;
        long size;	/* in bytes */
        LuciFileMode mode;
    } file;
    struct {
        struct LuciObject **items;
        int count;	/* current number of items in list */
        int size;	/* current count of allocated item pointers */
    } list;
    struct {
        struct LuciObject *list;
        uint32_t idx;
        uint32_t incr;
    } iterator;
    struct {
        void *frame;
        void (*deleter)(void *);
    } func;
    struct LuciObject * (*libfunc)(struct LuciObject **, int);
} LuciOVal;

/* LuciFunction is a type of function that returns a LuciObject * */
/* typedef struct LuciObject * (*LuciFunction) (struct LuciObject *); */

/* Generic Object which allows for dynamic typing */
typedef struct LuciObject
{
    LuciOType type;
    int refcount;
    LuciOVal value;
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
