/*
 * See Copyright Notice in luci.h
 */

/**
 * @file floattype.c
 */

#include "floattype.h"

/** Type member table for LuciFloatObj */
LuciObjectType obj_float_t = {
    "float",
    sizeof(LuciFloatObj),

    LuciFloat_copy,
    LuciFloat_copy,
    LuciFloat_repr,
    LuciFloat_asbool,
    unary_nil,
    LuciFloat_neg,
    LuciObject_lgnot,
    LuciFloat_bwnot,

    LuciFloat_add,
    LuciFloat_sub,
    LuciFloat_mul,
    LuciFloat_div,
    LuciFloat_mod,
    LuciFloat_pow,
    LuciFloat_eq,
    LuciFloat_neq,
    LuciFloat_lt,
    LuciFloat_gt,
    LuciFloat_lte,
    LuciFloat_gte,
    LuciObject_lgor,
    LuciObject_lgand,
    LuciFloat_bwxor,
    LuciFloat_bwor,
    LuciFloat_bwand,

    binary_nil,
    binary_nil,
    binary_nil,

    ternary_nil,

    LuciFloat_print,
    LuciFloat_mark,
    NULL,       /* finalize */
    NULL,       /* hash0 */
    NULL,       /* hash1 */
};

/**
 * Creates a new LuciFloatObj
 *
 * @param d double floating-point value
 * @returns new LuciFloatObj
 */
LuciObject *LuciFloat_new(double d)
{
    LuciFloatObj *o = (LuciFloatObj*)gc_malloc(&obj_float_t);
    o->f = d;
    return (LuciObject *)o;
}

/**
 * Copies a LuciFloatObj
 *
 * @param orig LucFloatObj to copy
 * @returns new copy of orig
 */
LuciObject* LuciFloat_copy(LuciObject *orig)
{
    return LuciFloat_new(((LuciFloatObj *)orig)->f);
}

/**
 * Produces the LuciStringObj representation of a LuciFloatObj
 *
 * @param o LuciFloatObj to represent
 * @returns LuciStringObj representation of o
 */
LuciObject* LuciFloat_repr(LuciObject *o)
{
    char *s = alloc(MAX_FLOAT_DIGITS);
    snprintf(s, MAX_FLOAT_DIGITS, "%f", (float)AS_FLOAT(o)->f);
    /* AS_STRING(ret)->s[16] = '\0'; */
    return LuciString_new(s);
}

/**
 * Returns boolean representation of a LuciFloatObj
 *
 * @param o LuciFloatObj
 * @returns true if o != 0.0
 */
LuciObject* LuciFloat_asbool(LuciObject *o)
{
    return LuciInt_new(AS_FLOAT(o)->f > 0.0L);
}

/**
 * Sum
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a + b
 */
LuciObject* LuciFloat_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f + AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f + AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot add object of type %s to a float\n", b->type->type_name);
    }
    return res;
}

/**
 * Difference
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a - b
 */
LuciObject* LuciFloat_sub(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f - AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f - AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot subtract an object of type %s from a float\n", b->type->type_name);
    }
    return res;
}

/**
 * Multiplied by
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a * b
 */
LuciObject* LuciFloat_mul(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f * AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f * AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot multiply an object of type %s and a float\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Divided by
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a / b
 */
LuciObject* LuciFloat_div(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        if (AS_INT(b)->i != 0) {
            res = LuciFloat_new(AS_FLOAT(a)->f / AS_INT(b)->i);
        } else {
            LUCI_DIE("%s\n", "Divide by zero");
        }
    } else if (ISTYPE(b, obj_float_t)) {
        if (AS_FLOAT(b)->f != 0.0L) {
            res = LuciFloat_new(AS_FLOAT(a)->f / AS_FLOAT(b)->f);
        } else {
            LUCI_DIE("%s\n", "Divide by zero");
        }
    } else {
        LUCI_DIE("Cannot divide a float by an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Modulus
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a % b
 */
LuciObject* LuciFloat_mod(LuciObject *a, LuciObject *b)
{
    LUCI_DIE("%s\n", "Cannot compute float modulus");
}

/**
 * Exponential
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a ** b
 */
LuciObject* LuciFloat_pow(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(pow(AS_FLOAT(a)->f, AS_INT(b)->i));
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(pow(AS_FLOAT(a)->f, AS_FLOAT(b)->f));
    } else {
        LUCI_DIE("Cannot compute the power of a float using an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Equal to
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a == b
 */
LuciObject* LuciFloat_eq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f == AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f == AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if a float is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Not equal to
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a != b
 */
LuciObject* LuciFloat_neq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f != AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f != AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if a float is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Less than
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a < b
 */
LuciObject* LuciFloat_lt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f < AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f < AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if a float is less than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Greater than
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a > b
 */
LuciObject* LuciFloat_gt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f > AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f > AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if a float is greater than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Less than or equal to
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a <= b
 */
LuciObject* LuciFloat_lte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f <= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f <= AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if a float is less than or equal "
                "to an object of type %s\n", b->type->type_name);
    }
    return res;
}

/**
 * Greater than or equal to
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a >= b
 */
LuciObject* LuciFloat_gte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f >= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f >= AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if a float is greater than or equal "
                "to an object of type %s\n", b->type->type_name);
    }
    return res;
}

/**
 * Bitwise-XOR
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a ^ b
 */
LuciObject* LuciFloat_bwxor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f ^ AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f ^ (long)AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Can't compute bitwise XOR of a float and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Bitwise-OR
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a | b
 */
LuciObject* LuciFloat_bwor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f | AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f | (long)AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Can't compute bitwise OR of a float and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Bitwise-AND
 *
 * @param a LuciFloatObj
 * @param b LuciObject
 * @returns a & b
 */
LuciObject* LuciFloat_bwand(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f & AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f & (long)AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Can't compute bitwise AND of a float and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Returns bitwise-not of LuciIntObj
 *
 * @param a LuciFloatObj
 * @returns bitwise-not of a
 */
LuciObject* LuciFloat_bwnot(LuciObject *a)
{
    return LuciInt_new(~((long)AS_FLOAT(a)->f));
}

/**
 * Negates a LuciFloatobj
 *
 * @param a LuciFloatObj
 * @returns negative a
 */
LuciObject *LuciFloat_neg(LuciObject *a)
{
    return LuciFloat_new(-(AS_FLOAT(a)->f));
}

/**
 * Prints a LuciFloatObj to stdout
 *
 * @param in LuciFloatObj to print
 */
void LuciFloat_print(LuciObject *in)
{
    printf("%f", AS_FLOAT(in)->f);
}

/**
 * Marks a LuciFloatObj as reachable
 *
 * @param in LuciFloatObj
 */
void LuciFloat_mark(LuciObject *in)
{
    GC_MARK(in);
}
