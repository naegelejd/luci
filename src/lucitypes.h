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
    uintptr_t type;     /**< pointer to type implementation (lowest 2 bits are used by GC) */
} LuciObject;

/** Object type virtual method table */
typedef struct LuciObjectType
{
    char *type_name;    /**< name of the type */
    uint32_t size;      /**< size of an instance */

    /* unary methods */
    LuciObject* (*copy)(LuciObject *);  /**< copy method */
    LuciObject* (*deepcopy)(LuciObject *);  /**< deep copy constructor */
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
    void (*mark)(LuciObject *);         /**< mark as reachable */
    void (*finalize)(LuciObject *);     /**< clean up dependencies */
    unsigned int (*hash0)(LuciObject *);    /**< object hash 1 */
    unsigned int (*hash1)(LuciObject *);    /**< object hash 2 */
} LuciObjectType;

#define TYPE_NAME(a) TYPEOF(a)->type_name
#define TYPE_SIZE(a) TYPEOF(a)->size
#define COPY(a) TYPEOF(a)->copy(a)
#define DEEPCOPY(a) TYPEOF(a)->deepcopy(a)
#define REPR(a) TYPEOF(a)->repr(a)
#define ASBOOL(a) TYPEOF(a)->asbool(a)
#define LEN(a) TYPEOF(a)->len(a)
#define NEG(a) TYPEOF(a)->neg(a)
#define LGNOT(a) TYPEOF(a)->lgnot(a)
#define BWNOT(a) TYPEOF(a)->bwnot(a)
#define ADD(a, b) TYPEOF(a)->add(a, b)
#define SUB(a, b) TYPEOF(a)->sub(a, b)
#define MUL(a, b) TYPEOF(a)->mul(a, b)
#define DIV(a, b) TYPEOF(a)->div(a, b)
#define MOD(a, b) TYPEOF(a)->mod(a, b)
#define POW(a, b) TYPEOF(a)->pow(a, b)
#define EQ(a, b) TYPEOF(a)->eq(a, b)
#define NEQ(a, b) TYPEOF(a)->neq(a, b)
#define LT(a, b) TYPEOF(a)->lt(a, b)
#define GT(a, b) TYPEOF(a)->gt(a, b)
#define LTE(a, b) TYPEOF(a)->lte(a, b)
#define GTE(a, b) TYPEOF(a)->gte(a, b)
#define LGOR(a, b) TYPEOF(a)->lgor(a, b)
#define LGAND(a, b) TYPEOF(a)->lgand(a, b)
#define BWXOR(a, b) TYPEOF(a)->bwxor(a, b)
#define BWOR(a, b) TYPEOF(a)->bwor(a, b)
#define BWAND(a, b) TYPEOF(a)->bwand(a, b)
#define CONTAINS(a, b) TYPEOF(a)->contains(a, b)
#define NEXT(a, b) TYPEOF(a)->next(a, b)
#define CGET(a, b) TYPEOF(a)->cget(a, b)
#define CPUT(a, b, c) TYPEOF(a)->cput(a, b, c)
#define PRINT(a) TYPEOF(a)->print(a)
#define MARK(a) TYPEOF(a)->mark(a)
#define FINALIZE(a) TYPEOF(a)->finalize(a)
#define HASH0(a) TYPEOF(a)->hash0(a)
#define HASH1(a) TYPEOF(a)->hash1(a)

/** returns 1 if the object's type is the given type, 0 otherwise */
#define ISTYPE(o, t) (TYPEOF(o) == &(t))
/** returns 1 if two objects have the same type, 0 otherwise */
#define TYPES_MATCH(a, b) (TYPEOF(a) == TYPEOF(b))
/** return pointer to type struct of object */
/* #define TYPEOF(o)   (o->type & 0xFFFFFFFFFFFFFFFCl) */
#define TYPEOF(o)   ((LuciObjectType*)((((LuciObject*)(o))->type) & -4))


LuciObject *LuciObject_lgand(LuciObject *, LuciObject *);
LuciObject *LuciObject_lgor(LuciObject *, LuciObject *);
LuciObject *LuciObject_lgnot(LuciObject *);

void unary_void(LuciObject *);
void binary_void(LuciObject *, LuciObject *);
void ternary_void(LuciObject *, LuciObject *, LuciObject *);
LuciObject* unary_nil(LuciObject *);
LuciObject* binary_nil(LuciObject *, LuciObject *);
LuciObject* ternary_nil(LuciObject *, LuciObject *, LuciObject *);

#include "gc.h"     /* for gc_malloc */
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

/** allows "LuciNilObj" to be used as a LuciObject* */
#define LuciNilObj &LuciNilInstance

#endif
