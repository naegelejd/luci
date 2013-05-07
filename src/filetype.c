/*
 * See Copyright Notice in luci.h
 */

/**
 * @file filetype.c
 */

#include "filetype.h"

/** Type member table for LuciFileObj */
LuciObjectType obj_file_t = {
    "file",
    FLAG_DEEP_COPY,
    sizeof(LuciFileObj),

    unary_nil,
    unary_nil,
    LuciFile_asbool,
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

    LuciFile_print
};

/**
 * Creates a new LuciFileObj
 *
 * @param fp C file pointer
 * @param size length of file
 * @param mode enumerated file mode
 * @returns new LuciFileObj
 */
LuciObject *LuciFile_new(FILE *fp, long size, file_mode mode)
{
    LuciFileObj *o = gc_malloc(sizeof(*o));
    SET_TYPE(o, obj_file_t);
    o->ptr = fp;
    o->mode = mode;
    o->size = size;
    return (LuciObject *)o;
}

LuciObject* LuciFile_asbool(LuciObject *o)
{
    LuciObject *res = LuciNilObj;

    if (AS_FILE(o)->ptr) {
        res = LuciInt_new(true);
    } else {
        res = LuciInt_new(false);
    }
    return res;
}

/**
 * Prints a representation of a LuciFileObj to stdout
 *
 * @param in LuciFileObj to print
 */
void LuciFile_print(LuciObject *in)
{
    switch (AS_FILE(in)->mode) {
        case f_read_m:
            printf("<file 'r'>");
            break;
        case f_write_m:
            printf("<file 'w'>");
            break;
        case f_append_m:
            printf("<file 'a'>");
            break;
        default:
            break;
    }
}

