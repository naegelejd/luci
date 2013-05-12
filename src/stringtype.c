/*
 * See Copyright Notice in luci.h
 */

/**
 * @file stringtype.c
 */

#include "stringtype.h"

static unsigned int string_hash_0(LuciObject *s);
static unsigned int string_hash_1(LuciObject *s);
static unsigned int string_hash_2(LuciObject *s);


/** Type member table for LuciStringObj */
LuciObjectType obj_string_t = {
    "string",
    sizeof(LuciStringObj),

    LuciString_copy,
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

    LuciString_print,

    LuciString_mark,     /* mark */
    LuciString_finalize,     /* finalize */

    string_hash_0,
    string_hash_1
};

/**
 * Creates a new LuciStringObj
 *
 * @param s C-string value
 * @returns new LuciStringObj
 */
LuciObject *LuciString_new(char *s)
{
    LuciStringObj *o = (LuciStringObj*)gc_malloc(&obj_string_t);
    o->s = s;   /* not a copy! */
    o->len = strlen(o->s);
    return (LuciObject *)o;
}

/**
 * Copies a LuciStringObj
 *
 * @param orig LucStringObj to copy
 * @returns new copy of orig
 */
LuciObject* LuciString_copy(LuciObject *orig)
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
LuciObject* LuciString_repr(LuciObject *o)
{
    int len = AS_STRING(o)->len + 1;
    char *s = alloc(len);
    strncpy(s, AS_STRING(o)->s, len);
    return LuciString_new(s);
}

LuciObject* LuciString_asbool(LuciObject *o)
{
    return LuciInt_new(AS_STRING(o)->len > 0);
}

/**
 * Returns the length of a LuciStringObj
 *
 * @param o LuciStringObj
 * @returns length of o
 */
LuciObject* LuciString_len(LuciObject *o)
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
LuciObject* LuciString_add(LuciObject *a, LuciObject *b)
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
LuciObject* LuciString_mul(LuciObject *a, LuciObject *b)
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
LuciObject* LuciString_eq(LuciObject *a, LuciObject *b)
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
LuciObject *LuciString_contains(LuciObject *str, LuciObject *o)
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
LuciObject *LuciString_next(LuciObject *str, LuciObject *idx)
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
LuciObject* LuciString_cget(LuciObject *a, LuciObject *b)
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
LuciObject* LuciString_cput(LuciObject *a, LuciObject *b, LuciObject *c)
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
void LuciString_print(LuciObject *in)
{
    printf("%s", AS_STRING(in)->s);
}


/**
 * Computes a hash of a LuciStringObj
 *
 * djb2 (hash(i) = hash(i - 1) * 33 + str[i])
 *
 * @param s LuciStringObj to hash
 * @returns unsigned integer hash
 */
static unsigned int string_hash_0(LuciObject *s)
{
    char *str = AS_STRING(s)->s;

    unsigned int h = 5381;
    int c;

    while ((c = *str++))
        h = ((h << 5) + h) + c;
    return h;
}

/**
 * Computes a hash of a LuciStringObj
 *
 * sdbm (hash(i) = hash(i - 1) * 65599 + str[i])
 *
 * @param s LuciStringObj to hash
 * @returns unsigned integer hash
 */
static unsigned int string_hash_1(LuciObject *s)
{
    char *str = AS_STRING(s)->s;

    unsigned int h = 0;
    int c;
    while ((c = *str++))
        h = c + (h << 6) + (h << 16) - h;
    return h;
}

/**
 * Computes a hash of a LuciStringObj
 *
 * One-at-a-time (Bob Jenkins)
 *
 * @param s LuciStringObj to hash
 * @returns unsigned integer hash
 */
static unsigned int string_hash_2(LuciObject *s)
{
    char *str = AS_STRING(s)->s;

    unsigned int h = 0;
    int c;
    while ((c = *str++)) {
        h += c;
        h += h << 10;
        h ^= h >> 6;
    }
    h += h << 3;
    h ^= h >> 11;
    h += h << 15;
    return h;
}

void LuciString_mark(LuciObject *in)
{
    GC_MARK(in);
}

void LuciString_finalize(LuciObject *in)
{
    free(AS_STRING(in)->s);
}
