/*
 * See Copyright Notice in luci.h
 */

/**
 * @file lucitypes.c
 */

#include "luci.h"
#include "lucitypes.h"


static LuciObject* LuciNil_copy(LuciObject *);
static LuciObject* LuciNil_asbool(LuciObject *);
static void LuciNil_print(LuciObject *);

/** Type member table for LuciNilObj */
LuciObjectType obj_nil_t = {
    "nil",
    sizeof(LuciNilObj),

    LuciNil_copy,
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


/** Definition of LuciNilObj */
LuciObject LuciNilInstance = {
    &obj_nil_t
};

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
 * Returns false
 *
 * @param o LuciNilObj
 * @returns false
 */
static LuciObject* LuciNil_asbool(LuciObject *o)
{
    return LuciInt_new(false);
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
 * Returns logical not of any LuciObject
 *
 * @param o LuciObject
 * @returns logical-not of o
 */
LuciObject *LuciObject_lgnot(LuciObject *o)
{
    LuciObject *b = o->type->asbool(o);
    return LuciInt_new(!(AS_INT(b)->i));
}

/**
 * Returns logical and of any two LuciObjects
 *
 * @param a LuciObject
 * @param b LuciObject
 * @returns logical-and of a and b
 */
LuciObject *LuciObject_lgand(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = b->type->asbool(b);
    return LuciInt_new(AS_INT(a0)->i && AS_INT(b0)->i);
}

/**
 * Returns logical or of any two LuciObjects
 *
 * @param a LuciObject
 * @param b LuciObject
 * @returns logical-and of a and b
 */
LuciObject *LuciObject_lgor(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = b->type->asbool(b);
    return LuciInt_new(AS_INT(a0)->i || AS_INT(b0)->i);
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
