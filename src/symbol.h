/*
 * See Copyright Notice in luci.h
 */

/**
 * @file symbol.h
 */

#ifndef SYMBOL_H
#define SYMBOL_H

#include "lucitypes.h"

/**
 * A linked-list node containing a node and index (reference)
 * to its corresponding SymbolTable object array
 */
typedef struct symbol
{
    struct symbol *next;    /**< next symbol in linked list */
    char *name;       /**< symbol name */
    unsigned int index;         /**< index of object corresponding to symbol */
} Symbol;

/**
 * A hash table implementation for storing symbols
 * during compile time
 */
typedef struct symtable
{
    Symbol **symbols;       /**< Symbol array */
    LuciObject **objects;   /**< Object array */
    unsigned int count;     /**< Current # of allocated symbol/object pairs */
    unsigned int size;      /**< Total object array size */
    unsigned int collisions;/**< Current # of collisions in hashtable */
    unsigned int bscale;    /**< Index into array of bucket size options */
    bool owns_objects;      /**< Whether the symtable can free objects */
} SymbolTable;

/** flags identifying the symtable_id intent (create/lookup/...) */
typedef enum { SYMFIND = 0x0, SYMCREATE = 0x1 } symtable_flags;

SymbolTable *symtable_new(unsigned int);
void symtable_delete(SymbolTable *);
void symtable_set(SymbolTable *, LuciObject *, unsigned int id);
int symtable_id(SymbolTable *, const char *, symtable_flags flags);

LuciObject **symtable_copy_objects(SymbolTable *);
LuciObject **symtable_give_objects(SymbolTable *);

#endif
