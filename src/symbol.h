/*
 * See Copyright Notice in luci.h
 */

#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdint.h>
#include "object.h"

/* A name and either a LuciObject or a function */
typedef struct symbol
{
    enum { sym_bobj_t, sym_bfunc_t, sym_uobj_t, sym_ufunc_t } type;
    const char *name;
    uint32_t index;
    /*
    union
    {
	struct LuciObject * object;
	LuciFunction funcptr;
	void *user_defined;
    } data;
    */
    struct symbol *next;
} Symbol;

typedef struct symtable
{
    uint8_t owns_objects; /* boolean. Affects deallocation of Table */
    uint32_t count;      /* Current # of allocated symbol/object pairs */
    uint32_t bscale;     /* Index into array of bucket size options (symbol.c) */
    uint32_t collisions; /* Current # of collisions in hashtable */
    uint32_t size;       /* Total object array size */
    Symbol **symbols;   /* Symbol array */
    LuciObject **objects;   /* Object array */
} SymbolTable;

#define SYMFIND     0x0
#define SYMCREATE   0x1

SymbolTable *symtable_new(uint32_t);
void symtable_delete(SymbolTable *);
int symtable_id(SymbolTable *, const char *, uint8_t flags);
void symtable_set(SymbolTable *, LuciObject *, uint32_t);

LuciObject **symtable_get_objects(SymbolTable *);

#endif
