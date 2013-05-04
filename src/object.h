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
#define MAKE_INDEX_POS(idx, len) while ((idx) < 0) { (idx) = (len) + (idx); }

/** Generic Object which allows for dynamic typing */
typedef struct LuciObject_ {
    struct LuciObjectType *type;    /**< pointer to type implementation */
} LuciObject;

/** type flag bits */
enum {
    FLAG_DEEP_COPY = 0,
    FLAG_SHALLOW_COPY = 1,
};

/** Object type virtual method table */
typedef struct LuciObjectType
{
    char *type_name;    /**< name of the type */
    uint32_t flags;     /**< type flags */
    uint32_t size;      /**< size of an instance */

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
typedef struct LuciIntObj_
{
    LuciObject base;    /**< base implementation */
    long i;             /**< 'long' numerical value */
} LuciIntObj;

/** Floating-point object type */
typedef struct LuciFloatObj_
{
    LuciObject base;    /**< base implementation */
    double f;           /**< double-precision floating point value */
} LuciFloatObj;

/** String object type */
typedef struct LuciString_ {
    LuciObject base;    /**< base implementation */
    char * s;           /**< pointer to C-string */
    long len;           /**< string length */
} LuciStringObj;

/** File open type */
typedef enum { f_read_m, f_write_m, f_append_m } file_mode;

/** File object type */
typedef struct LuciFile_ {
    LuciObject base;    /**< base implementation */
    FILE * ptr;         /**< pointer to C-file pointer */
    long size;          /**< file length in bytes */
    file_mode mode;     /**< current mode file was opened in */
} LuciFileObj;

/** List object type */
typedef struct LuciList_ {
    LuciObject base;    /**< base implementation */
    LuciObject **items; /**< pointer to items array */
    unsigned int count;	/**< current number of items in list */
    unsigned int size;	/**< current count of allocated items */
} LuciListObj;

/** Map object type */
typedef struct LuciMap_ {
    LuciObject base;    /**< base implementation */
    LuciObject **keys;  /**< array of pointers to keys */
    LuciObject **vals;  /**< array of pointers to values */
    unsigned int size_idx;  /**< identifier for current size of table */
    unsigned int collisions;    /**< number of hash collisions */
    unsigned int count;	/**< current number of key/value pairs */
    unsigned int size;	/**< current count of allocated pairs*/
} LuciMapObj;

/** Iterator object type (internal) */
typedef struct LuciIterator_ {
    LuciObject base;        /**< base implemenatation */
    LuciObject *idx;        /**< current index */
    int step;               /**< amount to increment by */
    LuciObject *container;  /**< the container this iterator applies to */
} LuciIteratorObj;

/** User-defined function type */
typedef struct LuciFunction_ {
    LuciObject base;            /**< base implementation */
    Instruction *instructions;  /**< array of instructions */
    Instruction *ip;            /**< current instruction pointer */
    LuciObject **locals;        /**< array of local LuciObjects */
    LuciObject **globals;       /**< array of global LuciObjects */
    LuciObject **constants;     /**< array of constant LuciObjects */
    uint32_t ninstrs;           /**< total number of instructions */
    uint16_t nparams;           /**< number of parameters */
    uint16_t nlocals;           /**< number of local symbols */
    uint16_t nconstants;        /**< number of constants */
} LuciFunctionObj;


/** Function pointer signature for Luci's builtin library functions */
typedef LuciObject* (*LuciCFunc)(LuciObject **, unsigned int);

/** Library function type */
typedef struct LuciLibFunc_ {
    LuciObject base;        /**< base implementation */
    LuciCFunc func;         /**< pointer to a Luci C function */
    char *help;             /**< help string for Luci C function */
    int min_args;           /**< minimum number of arguments to function */
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
/** casts LuciObject o to a LuciFunctionObj */
#define AS_FUNCTION(o)  ((LuciFunctionObj *)(o))
/** casts LuciObject o to a LuciLibFuncObj */
#define AS_LIBFUNC(o)  ((LuciLibFuncObj *)(o))


LuciObject *LuciInt_new(long l);
LuciObject *LuciFloat_new(double d);
LuciObject *LuciString_new(char *s);
LuciObject *LuciFile_new(FILE *fp, long size, file_mode mode);
LuciObject *LuciList_new();
LuciObject *LuciMap_new();
LuciObject *LuciIterator_new(LuciObject *list, int step);
LuciObject *LuciFunction_new();
LuciObject *LuciLibFunc_new(LuciCFunc fptr, char *help, int min_args);

unsigned int string_hash_0(LuciObject *s);
unsigned int string_hash_1(LuciObject *s);
unsigned int string_hash_2(LuciObject *s);

LuciObject *iterator_next_object(LuciObject *iterator);

void unary_void(LuciObject *);
void binary_void(LuciObject *, LuciObject *);
void ternary_void(LuciObject *, LuciObject *, LuciObject *);
LuciObject* unary_nil(LuciObject *);
LuciObject* binary_nil(LuciObject *, LuciObject *);
LuciObject* ternary_nil(LuciObject *, LuciObject *, LuciObject *);

#endif
