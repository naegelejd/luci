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
    sizeof(LuciFileObj),

    LuciFile_copy,
    LuciFile_copy,
    LuciFile_repr,
    LuciFile_asbool,
    LuciFile_len,
    unary_nil,
    LuciObject_lgnot,
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
    LuciObject_lgor,
    LuciObject_lgand,
    binary_nil,
    binary_nil,
    binary_nil,

    binary_nil,
    binary_nil,
    binary_nil,

    ternary_nil,

    LuciFile_print,
    LuciFile_mark,
    LuciFile_finalize,
    NULL,       /* hash0 */
    NULL        /* hash1 */
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
    LuciFileObj *o = (LuciFileObj*)gc_malloc(&obj_file_t);
    o->ptr = fp;
    o->mode = mode;
    o->size = size;
    o->iscopy = false;
    return (LuciObject *)o;
}

/**
 * Copies a LuciFileObj
 *
 * Marks the copy's 'iscopy' flag to ensure garbage collection
 * never closes a FILE* that is supposed to be open
 *
 * @param o LuciFileObj to copy
 * @returns copy of LuciFileObj
 */
LuciObject *LuciFile_copy(LuciObject *o)
{
    LuciFileObj *f = AS_FILE(o);
    LuciObject *copy = LuciFile_new(f->ptr, f->size, f->mode);
    AS_FILE(copy)->iscopy = true;    /* prevent garbage collector from closing this file */
    return copy;
}

/**
 * Produces a LuciStringObj representation of a LuciFileObj
 *
 * @param o LuciFileObj to represent
 * @returns LuciStringObj representation of o
 */
LuciObject *LuciFile_repr(LuciObject *o)
{
    char *s = alloc(32);
    snprintf(s, 32, "file @ %p", AS_FILE(o)->ptr);
    return LuciString_new(s);
}

/**
 * Returns a boolean representation of a LuciFileObj
 *
 * true if the file pointer is not NULL
 *
 * @returns LuciIntObj
 */
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
 * Returns the size of a LuciFileObj in bytes
 *
 * @param o LuciFileObj
 * @returns LuciIntObj size of file in bytes
 */
LuciObject* LuciFile_len(LuciObject *o)
{
    return LuciInt_new(AS_FILE(o)->size);
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

/**
 * Marks a LuciFileObj as reachable
 *
 * @param in LuciFileObj
 */
void LuciFile_mark(LuciObject *in)
{
    GC_MARK(in);
}

/**
 * Finalizes a LuciFileObj
 *
 * Closes it's internal C FILE* only if it is not a copy of
 * the original LuciFileObj
 *
 * @param in LuciFileObj
 */
void LuciFile_finalize(LuciObject *in)
{
    LuciFileObj *f = AS_FILE(in);

    if (!f->iscopy) {
        fclose(f->ptr);
    }
}
