/*
 * See Copyright Notice in luci.h
 */

/**
 * @file lucitypes.h
 *
 */

#ifndef LUCITYPES_H
#define LUCITYPES_H

#include "luci.h"
#include "gc.h"

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

/** convenient method of accessing an object's type functions */
#define MEMBER(o,m) (((LuciObject *)(o))->type->(##m))
/** returns 1 if the object's type is the given type, 0 otherwise */
#define ISTYPE(o,t) (((LuciObject *)(o))->type == (&(t)))
/** sets the type of the given object to the given type */
#define SET_TYPE(o, t)  (((LuciObject *)(o))->type = (&(t)))
/** returns 1 if two objects have the same type, 0 otherwise */
#define TYPES_MATCH(left, right) ((left)->type == (right)->type)


void unary_void(LuciObject *);
void binary_void(LuciObject *, LuciObject *);
void ternary_void(LuciObject *, LuciObject *, LuciObject *);
LuciObject* unary_nil(LuciObject *);
LuciObject* binary_nil(LuciObject *, LuciObject *);
LuciObject* ternary_nil(LuciObject *, LuciObject *, LuciObject *);

#include "inttype.h"
#include "floattype.h"
#include "stringtype.h"
#include "listtype.h"
#include "maptype.h"
#include "functiontype.h"
#include "iteratortype.h"
#include "filetype.h"
#include "nativefunctype.h"

extern LuciObjectType obj_nil_t;
extern LuciObject LuciNilInstance;
#define LuciNilObj &LuciNilInstance

#endif
