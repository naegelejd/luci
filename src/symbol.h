#ifndef SYMBOL_H
#define SYMBOL_H

#include "object.h"

/* A name and either a LuciObject or a function */
typedef struct symbol
{
    enum { sym_bobj_t, sym_bfunc_t, sym_uobj_t, sym_ufunc_t } type;
    const char *name;
    int index;
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
    int count;      /* Current # of allocated symbol/object pairs */
    int buckets;    /* Total # of buckets in hashtable */
    int collisions; /* Current # of collisions in hashtable */
    int size;       /* Total object array size */
    Symbol **symbols;   /* Symbol array */
    LuciObject **objects;   /* Object array */
} SymbolTable;

SymbolTable *symtable_new(int);
void symtable_delete(SymbolTable *);
int symbol_id(SymbolTable *, const char *);
void symtable_set(SymbolTable *, LuciObject *, int);
LuciObject *symtable_get(SymbolTable *, int);

#endif
