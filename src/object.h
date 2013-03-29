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


/** Generic Object which allows for dynamic typing */
typedef struct _LuciObject
{
    struct LuciObjectType *type;    /**< pointer to type implementation */
} LuciObject;

/** Object type virtual method table */
typedef struct LuciObjectType
{
    char *type_name;                    /**< name of the type */

    /* unary methods */
    LuciObject* (*copy)(LuciObject *);  /**< copy constructor */
    LuciObject* (*repr)(LuciObject *);  /**< LuciStringObj representation */
    LuciObject* (*asbool)(LuciObject *);/**< 0 or 1 representation */
    LuciObject* (*len)(LuciObject *);   /**< length/size of container */

    LuciObject* (*neg)(LuciObject *);   /**< negated object */
    LuciObject* (*lgnot)(LuciObject *); /**< lgnot */
    LuciObject* (*bwnot)(LuciObject *); /**< bnot */

    /* binary methods */
    LuciObject* (*add)(LuciObject *, LuciObject *); /**< add */
    LuciObject* (*sub)(LuciObject *, LuciObject *); /**< sub */
    LuciObject* (*mul)(LuciObject *, LuciObject *); /**< mul */
    LuciObject* (*div)(LuciObject *, LuciObject *); /**< div */
    LuciObject* (*mod)(LuciObject *, LuciObject *); /**< mod */
    LuciObject* (*pow)(LuciObject *, LuciObject *); /**< pow */
    LuciObject* (*eq)(LuciObject *, LuciObject *); /**< eq */
    LuciObject* (*neq)(LuciObject *, LuciObject *); /**< neq */
    LuciObject* (*lt)(LuciObject *, LuciObject *); /**< lt */
    LuciObject* (*gt)(LuciObject *, LuciObject *); /**< gt */
    LuciObject* (*lte)(LuciObject *, LuciObject *); /**< lte */
    LuciObject* (*gte)(LuciObject *, LuciObject *); /**< gte */
    LuciObject* (*lgor)(LuciObject *, LuciObject *); /**< lgor */
    LuciObject* (*lgand)(LuciObject *, LuciObject *); /**< lgand */
    LuciObject* (*bwxor)(LuciObject *, LuciObject *); /**< bxor */
    LuciObject* (*bwor)(LuciObject *, LuciObject *); /**< bor */
    LuciObject* (*bwand)(LuciObject *, LuciObject *); /**< band */

    LuciObject* (*contains)(LuciObject *, LuciObject *); /**< contains */
    LuciObject* (*next)(LuciObject *, LuciObject *);    /**< next item */
    LuciObject* (*cget)(LuciObject *, LuciObject *); /**< get item */

    LuciObject* (*cput)(LuciObject *, LuciObject *, LuciObject *); /**< put item */
    void (*print)(LuciObject *);        /**< print to stdout */
    unsigned int (*hash0)(LuciObject *);    /**< object hash 1 */
    unsigned int (*hash1)(LuciObject *);    /**< object hash 2 */
} LuciObjectType;

extern LuciObjectType obj_nil_t;
extern LuciObjectType obj_int_t;
extern LuciObjectType obj_float_t;
extern LuciObjectType obj_string_t;
extern LuciObjectType obj_list_t;
extern LuciObjectType obj_map_t;
extern LuciObjectType obj_iterator_t;
extern LuciObjectType obj_file_t;
extern LuciObjectType obj_func_t;
extern LuciObjectType obj_libfunc_t;


extern LuciObject LuciNilInstance;
#define LuciNilObj &LuciNilInstance

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
    LuciObject base;        /**< base implemenatation */
    LuciObject *idx;        /**< current index */
    int step;               /**< amount to increment by */
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

/** convenient method of accessing an object's type functions */
#define MEMBER(o,m) (((LuciObject *)(o))->type->(##m))
/** returns 1 if the object's type is the given type, 0 otherwise */
#define ISTYPE(o,t) (((LuciObject *)(o))->type == (&(t)))
/** sets the type of the given object to the given type */
#define SET_TYPE(o, t)  (((LuciObject *)(o))->type = (&(t)))
/** returns 1 if two objects have the same type, 0 otherwise */
#define TYPES_MATCH(left, right) ((left)->type == (right)->type)

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
/** casts LuciObject o to a LuciIteratorObj */
#define AS_ITERATOR(o)  ((LuciIteratorObj *)(o))
/** casts LuciObject o to a LuciMapObj */
#define AS_FUNCTION(o)  ((LuciFunctionObj *)(o))


LuciObject *LuciInt_new(long l);
LuciObject *LuciFloat_new(double d);
LuciObject *LuciString_new(char *s);
LuciObject *LuciFile_new(FILE *fp, long size, file_mode mode);
LuciObject *LuciList_new();
LuciObject *LuciIterator_new(LuciObject *list, int step);
LuciObject *LuciFunction_new(void *frame);
LuciObject *LuciLibFunc_new(LuciObject * (*func)(LuciObject **, unsigned int));

unsigned int string_hash_0(LuciObject *s);
unsigned int string_hash_1(LuciObject *s);
unsigned int string_hash_2(LuciObject *s);

int list_append_object(LuciObject *list, LuciObject *item);
LuciObject *list_get_object(LuciObject *list, long index);
LuciObject *list_set_object(LuciObject *list, LuciObject *item, long index);
LuciObject *iterator_next_object(LuciObject *iterator);

void unary_void(LuciObject *);
void binary_void(LuciObject *, LuciObject *);
void ternary_void(LuciObject *, LuciObject *, LuciObject *);
static LuciObject* unary_nil(LuciObject *);
static LuciObject* binary_nil(LuciObject *, LuciObject *);
static LuciObject* ternary_nil(LuciObject *, LuciObject *, LuciObject *);

#endif
