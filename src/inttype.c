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
    LuciInt_lgnot,
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
    LuciInt_lgor,
    LuciInt_lgand,
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

LuciObject* LuciInt_asbool(LuciObject *o)
{
    return LuciInt_new(AS_INT(o)->i > 0L);
}

/**
 * Adds a LuciIntObj to a second numeric LuciObject
 *
 * If b is a LuciFloatObj, the sum is promoted to a LuciFloatObj
 *
 * @param a LuciIntObj
 * @param b LuciIntObj or LuciFloatObj
 * @returns sum as a LuciIntObj or LuciFloatObj
 */
LuciObject* LuciInt_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i + AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i + AS_FLOAT(b)->f);
    } else {
        DIE("Cannot add object of type %s to an int\n", b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_sub(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i - AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i - AS_FLOAT(b)->f);
    } else {
        DIE("Cannot subtract an object of type %s from an int\n", b->type->type_name);
    }
    return res;
}


LuciObject* LuciInt_mul(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i * AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_INT(a)->i * AS_FLOAT(b)->f);
    } else {
        DIE("Cannot multiply an object of type %s and an int\n", b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_div(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        if (AS_INT(b)->i != 0) {
            res = LuciInt_new(AS_INT(a)->i / AS_INT(b)->i);
        } else {
            DIE("%s\n", "Divide by zero");
        }
    } else if (ISTYPE(b, obj_float_t)) {
        if (AS_FLOAT(b)->f != 0.0L) {
            res = LuciFloat_new(AS_INT(a)->i / AS_FLOAT(b)->f);
        } else {
            DIE("%s\n", "Divide by zero");
        }
    } else {
        DIE("Cannot divide an int by an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_mod(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        if (AS_INT(b)->i != 0) {
            res = LuciInt_new(AS_INT(a)->i % AS_INT(b)->i);
        } else {
            DIE("%s\n", "Modulus divide by zero");
        }
    } else {
        DIE("Cannot compute int modulus using an object of type %s\n",
                b->type->type_name);
    }

    return res;
}

LuciObject* LuciInt_pow(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(pow(AS_INT(a)->i, AS_INT(b)->i));
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(pow(AS_INT(a)->i, AS_FLOAT(b)->f));
    } else {
        DIE("Cannot compute the power of an int using an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_eq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i == AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i == AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_neq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i != AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i != AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_lt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i < AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i < AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is less than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_gt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i > AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i > AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is greater than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_lte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i <= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i <= AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is less than or equal to an "
                "object of type %s\n", b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_gte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i >= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i >= AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if an int is greater than or equal to an "
                "object of type %s\n", b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_lgor(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = a->type->asbool(b);

    return LuciInt_new(AS_INT(a0)->i || AS_INT(b0)->i);
}

LuciObject* LuciInt_lgand(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = a->type->asbool(b);

    return LuciInt_new(AS_INT(a0)->i && AS_INT(b0)->i);
}

LuciObject* LuciInt_bwxor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i ^ AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i ^ (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise XOR of an int and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_bwor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i | AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i | (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise OR of an int and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_bwand(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_INT(a)->i & AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_INT(a)->i & (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise AND of an int and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

LuciObject* LuciInt_lgnot(LuciObject *a)
{
    LuciObject *a0 = a->type->asbool(a);
    return LuciInt_new(!(AS_INT(a0)->i));
}

LuciObject* LuciInt_bwnot(LuciObject *a)
{
    return LuciInt_new(~(AS_INT(a)->i));
}

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

void LuciInt_mark(LuciObject *in)
{
    GC_MARK(in);
}
