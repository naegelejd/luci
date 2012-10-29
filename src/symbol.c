#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "common.h"
#include "object.h"
#include "symbol.h"

static Symbol *symbol_new(const char *, int);
static void *symbol_delete(Symbol *);

static SymbolTable *symtable_insert(SymbolTable *, Symbol *);
static SymbolTable *symtable_resize(SymbolTable *, int);
static Symbol *find_symbol_by_name(SymbolTable *, const char *);

static uint32_t hash_symbol(SymbolTable *, const char *);
static uint32_t djb2(const char *);
static uint32_t sdbm(const char *);
static uint32_t oaat(const char *);

/*
 * hash(i) = hash(i - 1) * 33 + str[i]
 */
static uint32_t djb2(const char *str)
{
    uint32_t h = 5381;
    int c;

    while (c = *str++)
        h = ((h << 5) + h) + c;
        /* h = ((h << 5 - h)) + c;  // h * 31 + c */
    return h;
}

/*
 * hash(i) = hash(i - 1) * 65599 + str[i]
 */
static uint32_t sdbm(const char *str)
{
    uint32_t h = 0;
    int c;
    while (c = *str++)
        h = c + (h << 6) + (h << 16) - h;
    return h;
}

/*
 * One-at-a-time (Bob Jenkins)
 */
static uint32_t oaat(const char *str)
{
    uint32_t h = 0;
    int c;
    while (c = *str++) {
        h += c;
        h += h << 10;
        h ^= h >> 6;
    }
    h += h << 3;
    h ^= h >> 11;
    h += h << 15;
    return h;
}

static uint32_t (*hashfuncs[])(const char *str) = {
    djb2,
    sdbm,
    oaat,
};

static uint32_t hash_symbol(SymbolTable *symtable, const char *name)
{
    return hashfuncs[0](name) % symtable->buckets;
}

static Symbol *symbol_new(const char *str, int index)
{
    Symbol *new = malloc(sizeof(*new));
    new->name = str;
    new->index = index;
    new->next = NULL;
    return new;
}

static void *symbol_delete(Symbol *del)
{
    if (del) {
        if (del->name) {
            free((char *)del->name);
            del->name = NULL;
        }
        free(del);
        del = NULL;
    }
    return;
}

/**
 * Inserts new symbol into symbol table.
 * SymbolTable must not already contain symbol
 */
static SymbolTable *symtable_insert(SymbolTable *symtable, Symbol *new_symbol)
{
    Symbol *cur = NULL, *prev = NULL;

    /* resize symbol table if if is over half full */
    if (symtable->count > (symtable->buckets >> 2))
        symtable = symtable_resize(symtable, symtable->buckets << 2);

    /* calculate hash of the symbol's name */
    uint32_t hash = hash_symbol(symtable, new_symbol->name);

    cur = symtable->symbols[hash];

    /* if bucket is empty, stuff new symbol into it */
    if (!cur) {
        symtable->symbols[hash] = new_symbol;
        return;
    }
    /* walk through linked list */
    while (cur) {
        prev = cur;
        cur = cur->next;
        symtable->collisions ++;    /* count collisions for fun */
    }
    prev->next = new_symbol;   /* append it to the last node */

    return symtable;
}

static SymbolTable *symtable_resize(SymbolTable *symtable, int buckets)
{
    if (symtable->buckets <= buckets)
        return symtable;

    yak("Resizing symtable from %d to %d\n", symtable->buckets, buckets);

    /* save the old attrs */
    Symbol **old_symbols = symtable->symbols;
    int old_buckets = symtable->buckets;

    /* allocate new entry array and set bucket count */
    symtable->symbols = calloc(buckets, sizeof(*(symtable->symbols)));
    if (!symtable->symbols)
        die("Error allocating new, larger symtable entry array\n");
    symtable->buckets = buckets;

    Symbol *cur = NULL, *prev = NULL;
    Symbol *inserted = NULL;
    int i = 0;
    for (i = 0; i < old_buckets; i ++) {
        cur = old_symbols[i];
        while (cur) {
            prev = cur;
            /* re-insert symbol into new hash table */
            symtable_insert(symtable, cur);
            /* TODO: copy symbol payload to new symbol location */
            cur = cur->next;
        }
    }
    /* delete old symbol array */
    free(old_symbols);

    return symtable;
}

/**
 * Allocates and returns a new symbol table.
 */
SymbolTable *symtable_new(int buckets)
{
    SymbolTable *symtable = calloc(1, sizeof(*symtable));
    if (!symtable)
        die("Error allocating symbol table\n");
    yak("Allocated symbol table\n");

    symtable->buckets = buckets;
    symtable->symbols = calloc(buckets, sizeof(*(symtable->symbols)));
    if (!symtable->symbols)
        die("Error allocating symtable symbols array\n");
    yak("Allocated symbol table symbols array\n");

    /* make Symbol Table object array size = bucket count / 16 */
    symtable->size = buckets >> 4;
    symtable->objects = calloc(symtable->size, sizeof(*(symtable->objects)));
    if (!symtable->objects)
        die("Error allocating symtable objects array\n");
    yak("Allocated symbol table objects array\n");

    return symtable;
}

/**
 * Deallocates all memory allocated for Symbol Table.
 *
 * Should be used at the end of interpretation,
 * as all Objects referenced by symbols will also
 * be destroyed.
 */
void symtable_delete(SymbolTable *symtable)
{
    Symbol *cur = NULL, *prev = NULL;
    int i;
    /* deallocate all symbols in table */
    for (i = 0; i < symtable->buckets; i ++) {
        cur = symtable->symbols[i];
        while (cur) {
            prev = cur;
            cur = cur->next;
            symbol_delete(prev);
        }
    }
    /* deallocate all objects in table */
    for (i = 0; i < symtable->count; i ++) {
        if (symtable->objects[i])
            destroy_object(symtable->objects[i]);
    }

    free(symtable->symbols);
    symtable->symbols = NULL;
    free(symtable->objects);
    symtable->objects = NULL;
    free(symtable);
    symtable = NULL;

    return;
}

static Symbol *find_symbol_by_name(SymbolTable *symtable, const char *name)
{
    Symbol *cur = NULL, *prev = NULL;
    /* Compute hash and bound it by the table's # of buckets */
    uint32_t hash = hash_symbol(symtable, name);

    /* search for symbol in table */
    cur = symtable->symbols[hash];
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            /* Found existing matching string in linked list */
            return cur;
        } else {
            /* advance through linked list */
            cur = cur->next;
        }
    }
    return cur; /* NULL */
}

/**
 * Creates new symbol if it doesn't already exist.
 * Returns the ID of the symbol.
 */
int symbol_id(SymbolTable *symtable, const char *name)
{
    Symbol *new_symbol = NULL;
    uint32_t hash;

    if (!symtable)
        die("Symbol Table not allocated\n");

    /* Try to find symbol in table */
    new_symbol = find_symbol_by_name(symtable, name);
    /* if found, return it's index (ID) */
    if (new_symbol)
        return new_symbol->index;

    /* otherwise, we need to create the symbol and insert it */

    /* Increase count of Symbol/Object pairs */
    symtable->count ++;
    if (symtable->count >= symtable->size) {
        /* ensure object array is large enough */
        symtable->size <<= 1;
        symtable->objects = realloc(symtable->objects,
                symtable->size * sizeof(*(symtable->objects)));
    }
    /* create the new symbol */
    new_symbol = symbol_new(name, symtable->count);

    /* insert the new symbol */
    symtable_insert(symtable, new_symbol);

    /* return symbol's ID (object array index) */
    return symtable->count;
}

/**
 * Replaces the object in the table pertaining to ID
 */
void symtable_set(SymbolTable *symtable, LuciObject *obj, int ID)
{
    if ((ID >= symtable->count) || (ID < 0))
        die("Symbol ID out of bounds\n");

    symtable->objects[ID] = obj;
}

/**
 * Returns the object in the table pertaining to ID
 */
LuciObject *symtable_get(SymbolTable *symtable, int ID)
{
    if ((ID >= symtable->count) || (ID < 0))
        die("Symbol ID out of bounds\n");

    return symtable->objects[ID];
}


