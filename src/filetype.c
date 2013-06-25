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
    unary_nil,
    LuciFile_asbool,
    unary_nil,
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
    return (LuciObject *)o;
}

/**
 * Copies a LuciFileObj
 *
 * @returns copy of LuciFileObj
 */
LuciObject* LuciFile_copy(LuciObject *o)
{
    LuciFileObj *f = AS_FILE(o);

    LuciObject *res = LuciFile_new(f->ptr, f->size, f->mode);
    return res;
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
 * @param fobj LuciFileObj
 */
void LuciFile_mark(LuciObject *fobj)
{
    GC_MARK(fobj);
}

/**
 * Finalizes a LuciFileObj
 *
 * SHOULD close the FILE pointer, but as it stands,
 * a copied LuciFileObj points to the same FILE, so
 * it can't be closed unless it's references are counted
 *
 * @param fobj LuciFileObj
 */
void LuciFile_finalize(LuciObject *fobj)
{
    /* fclose(AS_FILE(fobj)->ptr); */
}
