#include "luci.h"
#include "object.h"


static LuciObject* LuciString_copy(LuciObject *);
static LuciObject* LuciString_repr(LuciObject *);
static LuciObject* LuciString_asbool(LuciObject *);
static LuciObject* LuciString_len(LuciObject *);
static LuciObject* LuciString_add(LuciObject *, LuciObject *);
static LuciObject* LuciString_mul(LuciObject *, LuciObject *);
static LuciObject* LuciString_eq(LuciObject *, LuciObject *);
static LuciObject* LuciString_contains(LuciObject *m, LuciObject *o);
static LuciObject* LuciString_cget(LuciObject *, LuciObject *);
static LuciObject* LuciString_cput(LuciObject *, LuciObject *, LuciObject *);
static LuciObject* LuciString_next(LuciObject *, LuciObject *);
static void LuciString_print(LuciObject *);


/** Type member table for LuciStringObj */
LuciObjectType obj_string_t = {
    "string",
    LuciString_copy,
    LuciString_repr,
    LuciString_asbool,
    LuciString_len,
    unary_nil,
    unary_nil,
    unary_nil,

    LuciString_add,
    binary_nil,
    LuciString_mul,
    binary_nil,
    binary_nil,
    binary_nil,
    LuciString_eq,
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

    LuciString_contains,
    LuciString_next,
    LuciString_cget,

    LuciString_cput,

    LuciString_print
};


/**
 * Copies a LuciStringObj
 *
 * @param orig LucStringObj to copy
 * @returns new copy of orig
 */
static LuciObject* LuciString_copy(LuciObject *orig)
{
    return LuciString_new(strdup(((LuciStringObj *)orig)->s));
}

/**
 * Produces the LuciStringObj representation of a LuciStringObj
 *
 * While this may seem redundant, cast a string to a string
 * must return a *new* string.
 *
 * @param o LuciStringObj to represent
 * @returns LuciStringObj representation of o
 */
static LuciObject* LuciString_repr(LuciObject *o)
{
    int len = AS_STRING(o)->len + 1;
    char *s = alloc(len);
    strncpy(s, AS_STRING(o)->s, len);
    return LuciString_new(s);
}

static LuciObject* LuciString_asbool(LuciObject *o)
{
    return LuciInt_new(AS_STRING(o)->len > 0);
}

/**
 * Returns the length of a LuciStringObj
 *
 * @param o LuciStringObj
 * @returns length of o
 */
static LuciObject* LuciString_len(LuciObject *o)
{
    return LuciInt_new(AS_STRING(o)->len);
}


/**
 * Concatenates two LuciStringObjs
 *
 * @param a first LuciStringObj
 * @param b second LuciStringObj
 * @returns concatenated LuciStringObj
 */
static LuciObject* LuciString_add(LuciObject *a, LuciObject *b)
{
    if (ISTYPE(b, obj_string_t)) {
        char *s = alloc(AS_STRING(a)->len + AS_STRING(b)-> len + 1);
        strncpy(s, AS_STRING(a)->s, AS_STRING(a)->len);
        strncat(s, AS_STRING(b)->s, AS_STRING(b)->len);
        return LuciString_new(s);
    } else {
        DIE("Cannot append object of type %s to a string\n",
                b->type->type_name);
    }

    return LuciNilObj;
}

/**
 * Concatenates a LuciStringObj b times
 *
 * @param a LuciStringObj to multiply
 * @param b integer multiplier
 * @returns concatenated LuciStringObj
 */
static LuciObject* LuciString_mul(LuciObject *a, LuciObject *b)
{
    if (ISTYPE(b, obj_int_t)) {
        char *s = alloc(AS_STRING(a)->len * AS_INT(b)->i + 1);
        *s = '\0';

        int i;
        for (i = 0; i < AS_INT(b)->i; i++) {
            strncat(s, AS_STRING(a)->s, AS_STRING(a)->len);
        }
        return LuciString_new(s);
    } else {
        DIE("Cannot multiply a string by an object of type %s\n",
                b->type->type_name);
    }

    return LuciNilObj;
}

/**
 * Determines if two LuciStringObjs are equal
 *
 * @param a LuciStringObj
 * @param b LuciStringObj
 * @returns 1 if equal, 0 otherwise
 */
static LuciObject* LuciString_eq(LuciObject *a, LuciObject *b)
{
    if(ISTYPE(b, obj_string_t)) {
        if (AS_STRING(a)->len != AS_STRING(b)->len) {
            return LuciInt_new(false);
        }
        if ((strncmp(AS_STRING(a)->s, AS_STRING(b)->s, AS_STRING(a)->len)) == 0) {
            return LuciInt_new(true);
        } else {
            return LuciInt_new(false);
        }
    } else {
        DIE("Cannot compare a string to an object of type %s\n",
                b->type->type_name);
    }
    return LuciNilObj;
}

/**
 * Determines whether a LuciStringObj contains an object
 *
 * @param str LuciStringObj
 * @param o object
 * @returns 1 if str contains o, 0 otherwise
 */
static LuciObject *LuciString_contains(LuciObject *str, LuciObject *o)
{
    if (!ISTYPE(o, obj_string_t)) {
        DIE("A string can only contain a string, not a %s\n",
                o->type->type_name);
    }
    if ((strstr(AS_STRING(str)->s, AS_STRING(o)->s)) != NULL) {
        return LuciInt_new(true);
    } else {
        return LuciInt_new(false);
    }
}

/**
 * Returns the 'next' char in the string
 *
 * @param str LuciStringObj
 * @param idx index
 * @returns char at index idx or NULL if out of bounds
 */
static LuciObject *LuciString_next(LuciObject *str, LuciObject *idx)
{
    if (!ISTYPE(idx, obj_int_t)) {
        DIE("%s\n", "Argument to LuciString_next must be LuciIntObj");
    }

    if (AS_INT(idx)->i >= AS_STRING(str)->len) {
        return NULL;
    }

    char *s = alloc(2 * sizeof(char));
    s[0] = AS_STRING(str)->s[AS_INT(idx)->i];
    s[1] = '\0';
    return LuciString_new(s);
}

/**
 * Gets the character at index b in LuciStringObj a
 *
 * @param a LuciStringObj
 * @param b index in a
 * @returns character at index b
 */
static LuciObject* LuciString_cget(LuciObject *a, LuciObject *b)
{
    if (ISTYPE(b, obj_int_t)) {
        long idx = AS_INT(b)->i;

        MAKE_INDEX_POS(idx, AS_STRING(a)->len);

        if (idx >= AS_STRING(a)->len) {
            DIE("%s\n", "String subscript out of bounds");
        }

        char *s = alloc(2 * sizeof(char));
        s[0] = AS_STRING(a)->s[idx];
        s[1] = '\0';
        return LuciString_new(s);
    } else {
        DIE("Cannot subscript a string with an object of type %s\n",
                b->type->type_name);
    }
    return LuciNilObj;
}

/**
 * Sets the character at index b in LuciStringObj a
 *
 * @param a LuciStringObj
 * @param b index in a
 * @returns former character at index b
 */
static LuciObject* LuciString_cput(LuciObject *a, LuciObject *b, LuciObject *c)
{
    if (ISTYPE(b, obj_int_t)) {
        if (ISTYPE(c, obj_string_t)) {
            long idx = AS_INT(b)->i;
            MAKE_INDEX_POS(idx, AS_STRING(a)->len);
            if (idx >= AS_STRING(a)->len) {
                DIE("%s\n", "String subscript out of bounds");
            }
            char *s = alloc(2 * sizeof(char));
            s[0] = AS_STRING(a)->s[idx];
            s[1] = '\0';

            /* just put one char for now */
            AS_STRING(a)->s[idx] = AS_STRING(c)->s[0];

            /* return the former char */
            return LuciString_new(s);
        } else {
            DIE("Cannot put an object of type %s into a string\n", 
                    c->type->type_name);
        }
    } else {
        DIE("Cannot subscript a string with an object of type %s\n",
                b->type->type_name);
    }
    return LuciNilObj;
}

/**
 * Prints a LuciStringObj to stdout
 *
 * @param in LuciStringObj to print
 */
static void LuciString_print(LuciObject *in)
{
    printf("%s", AS_STRING(in)->s);
}
