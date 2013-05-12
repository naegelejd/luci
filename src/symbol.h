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
    const char *name;       /**< symbol name */
    uint32_t index;         /**< index of object corresponding to symbol */
} Symbol;

/**
 * A hash table implementation for storing symbols
 * during compile time
 */
typedef struct symtable
{
    Symbol **symbols;       /**< Symbol array */
    LuciObject **objects;   /**< Object array */
    uint32_t count;         /**< Current # of allocated symbol/object pairs */
    uint32_t size;          /**< Total object array size */
    uint32_t bscale;        /**< Index into array of bucket size options */
    uint32_t collisions;    /**< Current # of collisions in hashtable */
    uint8_t owns_objects;   /**< boolean. Affects deallocation of Table */
} SymbolTable;

/** flags identifying the symtable_id intent (create/lookup/...) */
typedef enum { SYMFIND = 0x0, SYMCREATE = 0x1 } symtable_flags;

SymbolTable *symtable_new(uint32_t);
void symtable_delete(SymbolTable *);
void symtable_set(SymbolTable *, LuciObject *, uint32_t id);
int symtable_id(SymbolTable *, const char *, symtable_flags flags);

LuciObject **symtable_get_objects(SymbolTable *);

#endif
