/*
 * See Copyright Notice in luci.h
 */

/**
 * @file inttype.c
 */

#include "inttype.h"


/** Type member table for LuciIntObj */
LuciObjectType obj_int_t = {
    "int",
    sizeof(LuciIntObj),

    LuciInt_copy,
    LuciInt_copy,
    LuciInt_repr,
    LuciInt_asbool,
    unary_nil,
    LuciInt_neg,
    LuciObject_lgnot,
    LuciInt_bwnot,

    LuciInt_add,
    LuciInt_sub,
    LuciInt_mul,
    LuciInt_div,
    LuciInt_mod,
    LuciInt_pow,
    LuciInt_eq,
    LuciInt_neq,
    LuciInt_lt,
    LuciInt_gt,
    LuciInt_lte,
    LuciInt_gte,
    LuciObject_lgor,
    LuciObject_lgand,
    LuciInt_bwxor,
    LuciInt_bwor,
    LuciInt_bwand,

    binary_nil,
    binary_nil,
    binary_nil,

    ternary_nil,

    LuciInt_print,

    LuciInt_mark,
    NULL,           /* finalize */
    NULL,           /* hash0 */
    NULL            /* hash1 */
};

/**
 * Creates a new LuciIntObj
 *
 * @param l long integer value
 * @returns new LuciIntObj
 */
LuciObject *LuciInt_new(long l)
{
    LuciIntObj *o = (LuciIntObj*)gc_malloc(&obj_int_t);
    o->i = l;
    return (LuciObject *)o;
}

/**
 * Copies a LuciIntObj
 *
 * @param orig LucIntObj to copy
 * @returns new copy of orig
 */
LuciObject* LuciInt_copy(LuciObject *orig)
{
    return LuciInt_new(((LuciIntObj *)orig)->i);
}

/**
 * Produces the LuciStringObj representation of a LuciIntObj
 *
 * @param o LuciIntObj to represent
 * @returns LuciStringObj representation of o
 */
LuciObject* LuciInt_repr(LuciObject *o)
{
    char *s = alloc(MAX_INT_DIGITS);
    snprintf(s, MAX_INT_DIGITS, "%ld", AS_INT(o)->i);
    /* ret->s[16] = '\0'; */
    return LuciString_new(s);
}

/**
 * Returns boolean representation of a LuciIntObj
 *
 * @param o LuciIntObj
 * @returns true if o != 0
 */
LuciObject* LuciInt_asbool(LuciObject *o)
{
    return LuciInt_new(AS_INT(o)->i > 0L);
}

/**
 * Sum
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a + b
 */
LuciObject* LuciInt_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i + AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i + AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot add object of type %s to an int\n", b->type->type_name);
    }
    return res;
}

/**
 * Difference
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a - b
 */
LuciObject* LuciInt_sub(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i - AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i - AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot subtract an object of type %s from an int\n", b->type->type_name);
    }
    return res;
}

/**
 * Multiplied by
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a * b
 */
LuciObject* LuciInt_mul(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i * AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i * AS_FLOAT(b)->f);
    } else if (ISTYPE(b, obj_string_t)) {
        res = b->type->mul(b, a);
    } else {
        LUCI_DIE("Cannot multiply an object of type %s and an int\n", b->type->type_name);
    }
    return res;
}

/**
 * Divided by
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a / b
 */
LuciObject* LuciInt_div(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        if (AS_INT(b)->i != 0) {
            res = LuciInt_new(AS_INT(a)->i / AS_INT(b)->i);
        } else {
            LUCI_DIE("%s\n", "Divide by zero");
        }
    } else if (ISTYPE(b, obj_float_t)) {
        if (AS_FLOAT(b)->f != 0.0L) {
            res = LuciFloat_new(AS_INT(a)->i / AS_FLOAT(b)->f);
        } else {
            LUCI_DIE("%s\n", "Divide by zero");
        }
    } else {
        LUCI_DIE("Cannot divide an int by an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Modulo
 *
 * Always returns a
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a % b
 */
LuciObject* LuciInt_mod(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        if (AS_INT(b)->i == 0) {
            LUCI_DIE("%s\n", "Modulo divide by zero");
        }
        long m = AS_INT(a)->i % AS_INT(b)->i;
        res = LuciInt_new(m);
    } else {
        LUCI_DIE("Cannot compute int modulo using an object of type %s\n",
                b->type->type_name);
    }

    return res;
}

/**
 * Exponential
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a ** b
 */
LuciObject* LuciInt_pow(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(pow(AS_INT(a)->i, AS_INT(b)->i));
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(pow(AS_INT(a)->i, AS_FLOAT(b)->f));
    } else {
        LUCI_DIE("Cannot compute the power of an int using an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Equal to
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a == b
 */
LuciObject* LuciInt_eq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i == AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i == AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if an int is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Not equal to
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a != b
 */
LuciObject* LuciInt_neq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i != AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i != AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if an int is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Less than
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a < b
 */
LuciObject* LuciInt_lt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i < AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i < AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if an int is less than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Greater than
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a > b
 */
LuciObject* LuciInt_gt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i > AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i > AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if an int is greater than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Less than or equal to
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a <= b
 */
LuciObject* LuciInt_lte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i <= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i <= AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if an int is less than or equal to an "
                "object of type %s\n", b->type->type_name);
    }
    return res;
}

/**
 * Greater than or equal to
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a >= b
 */
LuciObject* LuciInt_gte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i >= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i >= AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Cannot determine if an int is greater than or equal to an "
                "object of type %s\n", b->type->type_name);
    }
    return res;
}

/**
 * Bitwise-XOR
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a ^ b
 */
LuciObject* LuciInt_bwxor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i ^ AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i ^ (long)AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Can't compute bitwise XOR of an int and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Bitwise-OR
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a | b
 */
LuciObject* LuciInt_bwor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i | AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i | (long)AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Can't compute bitwise OR of an int and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Bitwise-AND
 *
 * @param a LuciIntObj
 * @param b LuciObject
 * @returns a & b
 */
LuciObject* LuciInt_bwand(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i & AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i & (long)AS_FLOAT(b)->f);
    } else {
        LUCI_DIE("Can't compute bitwise AND of an int and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

/**
 * Returns bitwise-not of LuciIntObj
 *
 * @param a LuciIntObj
 * @returns bitwise-not of a
 */
LuciObject* LuciInt_bwnot(LuciObject *a)
{
    return LuciInt_new(~(AS_INT(a)->i));
}

/**
 * Negates a LuciIntobj
 *
 * @param a LuciIntObj
 * @returns negative a
 */
LuciObject *LuciInt_neg(LuciObject *a)
{
    return LuciInt_new(-(AS_INT(a)->i));
}

/**
 * Prints a LuciIntObj to stdout
 *
 * @param in LuciIntObj to print
 */
void LuciInt_print(LuciObject *in)
{
    printf("%ld", AS_INT(in)->i);
}

/**
 * Marks a LuciIntObj as reachable
 *
 * @param in LuciIntObj
 */
void LuciInt_mark(LuciObject *in)
{
    GC_MARK(in);
}
