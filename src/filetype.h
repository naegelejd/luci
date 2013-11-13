/*
 * See Copyright Notice in luci.h
 */

/**
 * @file filetype.h
 */

#ifndef LUCI_FILETYPE_H
#define LUCI_FILETYPE_H

#include "lucitypes.h"

extern LuciObjectType obj_file_t;

/** File open type */
typedef enum { f_read_m, f_write_m, f_append_m } file_mode;

/** File object type */
typedef struct LuciFile_ {
    LuciObject base;    /**< base implementation */
    FILE * ptr;         /**< pointer to C-file pointer */
    long size;          /**< file length in bytes */
    file_mode mode;     /**< current mode file was opened in */
    bool iscopy;        /**< flag to prevent closing garbage collected copies */
} LuciFileObj;

/** casts LuciObject o to a LuciFileObj */
#define AS_FILE(o)      ((LuciFileObj *)(o))

LuciObject *LuciFile_new(FILE *fp, long size, file_mode mode);
LuciObject *LuciFile_copy(LuciObject *o);
LuciObject *LuciFile_repr(LuciObject *o);
LuciObject* LuciFile_asbool(LuciObject *);
LuciObject* LuciFile_len(LuciObject *o);
void LuciFile_print(LuciObject *);
void LuciFile_mark(LuciObject *in);
void LuciFile_finalize(LuciObject *in);


#endif
