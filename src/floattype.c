#include "luci.h"
#include "object.h"


static LuciObject* LuciFloat_copy(LuciObject *);
static LuciObject* LuciFloat_repr(LuciObject *);
static LuciObject* LuciFloat_asbool(LuciObject *);
static LuciObject* LuciFloat_add(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_sub(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_mul(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_div(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_mod(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_pow(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_eq(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_neq(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_lt(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_gt(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_lte(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_gte(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_lgor(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_lgand(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_bwxor(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_bwor(LuciObject *, LuciObject *);
static LuciObject* LuciFloat_bwand(LuciObject *, LuciObject *);

static LuciObject* LuciFloat_neg(LuciObject *);
static LuciObject* LuciFloat_lgnot(LuciObject *);
static LuciObject* LuciFloat_bwnot(LuciObject *);

static void LuciFloat_print(LuciObject *);


/** Type member table for LuciFloatObj */
LuciObjectType obj_float_t = {
    "float",
    DEEP_COPIED,
    LuciFloat_copy,
    LuciFloat_repr,
    LuciFloat_asbool,
    unary_nil,
    LuciFloat_neg,
    LuciFloat_lgnot,
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
    LuciFloat_lgor,
    LuciFloat_lgand,
    LuciFloat_bwxor,
    LuciFloat_bwor,
    LuciFloat_bwand,

    binary_nil,
    binary_nil,
    binary_nil,

    ternary_nil,

    LuciFloat_print
};


/**
 * Copies a LuciFloatObj
 *
 * @param orig LucFloatObj to copy
 * @returns new copy of orig
 */
static LuciObject* LuciFloat_copy(LuciObject *orig)
{
    return LuciFloat_new(((LuciFloatObj *)orig)->f);
}

/**
 * Produces the LuciStringObj representation of a LuciFloatObj
 *
 * @param o LuciFloatObj to represent
 * @returns LuciStringObj representation of o
 */
static LuciObject* LuciFloat_repr(LuciObject *o)
{
    char *s = alloc(MAX_FLOAT_DIGITS);
    snprintf(s, MAX_FLOAT_DIGITS, "%f", (float)AS_FLOAT(o)->f);
    /* AS_STRING(ret)->s[16] = '\0'; */
    return LuciString_new(s);
}

static LuciObject* LuciFloat_asbool(LuciObject *o)
{
    return LuciInt_new(AS_FLOAT(o)->f > 0.0L);
}

/**
 * Adds a LuciFloatObj to a second numeric LuciObject
 *
 * @param a LuciFloatObj
 * @param b LuciIntObj or LuciFloatObj
 * @returns sum as a LuciFloatObj
 */
static LuciObject* LuciFloat_add(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f + AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f + AS_FLOAT(b)->f);
    } else {
        DIE("Cannot add object of type %s to a float\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_sub(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f - AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f - AS_FLOAT(b)->f);
    } else {
        DIE("Cannot subtract an object of type %s from a float\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_mul(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f * AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(AS_FLOAT(a)->f * AS_FLOAT(b)->f);
    } else {
        DIE("Cannot multiply an object of type %s and a float\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_div(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        if (AS_INT(b)->i != 0) {
            res = LuciFloat_new(AS_FLOAT(a)->f / AS_INT(b)->i);
        } else {
            DIE("%s\n", "Divide by zero");
        }
    } else if (ISTYPE(b, obj_float_t)) {
        if (AS_FLOAT(b)->f != 0.0L) {
            res = LuciFloat_new(AS_FLOAT(a)->f / AS_FLOAT(b)->f);
        } else {
            DIE("%s\n", "Divide by zero");
        }
    } else {
        DIE("Cannot divide a float by an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_mod(LuciObject *a, LuciObject *b)
{
    DIE("%s\n", "Cannot compute float modulus");
}

static LuciObject* LuciFloat_pow(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciFloat_new(pow(AS_FLOAT(a)->f, AS_INT(b)->i));
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciFloat_new(pow(AS_FLOAT(a)->f, AS_FLOAT(b)->f));
    } else {
        DIE("Cannot compute the power of a float using an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_eq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f == AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f == AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_neq(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f != AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f != AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is equal to an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_lt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f < AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f < AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is less than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_gt(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f > AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f > AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is greater than an object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_lte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f <= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f <= AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is less than or equal "
                "to an object of type %s\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_gte(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f >= AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new(AS_FLOAT(a)->f >= AS_FLOAT(b)->f);
    } else {
        DIE("Cannot determine if a float is greater than or equal "
                "to an object of type %s\n", b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_lgor(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = a->type->asbool(a);

    return LuciInt_new(AS_INT(a0)->i || AS_INT(b0)->i);
}

static LuciObject* LuciFloat_lgand(LuciObject *a, LuciObject *b)
{
    LuciObject *a0 = a->type->asbool(a);
    LuciObject *b0 = a->type->asbool(a);

    return LuciInt_new(AS_INT(a0)->i && AS_INT(b0)->i);
}

static LuciObject* LuciFloat_bwxor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f ^ AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f ^ (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise XOR of a float and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_bwor(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f | AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f | (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise OR of a float and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_bwand(LuciObject *a, LuciObject *b)
{
    LuciObject *res = LuciNilObj;

    if (ISTYPE(b, obj_int_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f & AS_INT(b)->i);
    } else if (ISTYPE(b, obj_float_t)) {
        res = LuciInt_new((long)AS_FLOAT(a)->f & (long)AS_FLOAT(b)->f);
    } else {
        DIE("Can't compute bitwise AND of a float and object of type %s\n",
                b->type->type_name);
    }
    return res;
}

static LuciObject* LuciFloat_lgnot(LuciObject *a)
{
    LuciObject *a0 = a->type->asbool(a);
    return LuciInt_new(!(AS_INT(a0)->i));
}

static LuciObject* LuciFloat_bwnot(LuciObject *a)
{
    return LuciInt_new(~((long)AS_FLOAT(a)->f));
}

static LuciObject *LuciFloat_neg(LuciObject *a)
{
    return LuciFloat_new(-(AS_FLOAT(a)->f));
}

/**
 * Prints a LuciFloatObj to stdout
 *
 * @param in LuciFloatObj to print
 */
static void LuciFloat_print(LuciObject *in)
{
    printf("%f", AS_FLOAT(in)->f);
}

