/*
 * See Copyright Notice in luci.h
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "luci.h"
#include "common.h"
#include "object.h"
#include "symbol.h"

static Symbol *symbol_new(const char *, uint32_t);
static void *symbol_delete(Symbol *);

static SymbolTable *symtable_insert(SymbolTable *, Symbol *);
static SymbolTable *symtable_resize(SymbolTable *, uint32_t);
static Symbol *find_symbol_by_name(SymbolTable *, const char *);

static uint32_t hash_symbol(SymbolTable *, const char *);
static uint32_t djb2(const char *);
static uint32_t sdbm(const char *);
static uint32_t oaat(const char *);

/* http://planetmath.org/GoodHashTablePrimes.html */
enum { N_BUCKET_OPTIONS = 26 };
static unsigned int NBUCKETS[N_BUCKET_OPTIONS] = {
    97, 193, 389, 769, 1543, 3079, 6151, 12289,
    24593, 49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189,
    805306457, 1610612741, 0
};

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
    return hashfuncs[0](name) % NBUCKETS[symtable->bscale];
}

static Symbol *symbol_new(const char *str, uint32_t index)
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
        /*
         * The original creator of the 'char *'
         * which the symbol represents should
         * cleanup the memory allocated for
         * the 'char *'
         */
        /*
        if (del->name) {
            free((char *)del->name);
            del->name = NULL;
        }
        */
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
    if (symtable->count > (NBUCKETS[symtable->bscale] >> 1))
        symtable = symtable_resize(symtable, symtable->bscale++);

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

static SymbolTable *symtable_resize(SymbolTable *symtable, uint32_t bucketscale)
{
    /* no shrink implementation defined */
    if (symtable->bscale >= bucketscale)
        return symtable;

    LUCI_DEBUG("Resizing symtable from %d to %d\n",
            NBUCKETS[symtable->bscale], NBUCKETS[bucketscale]);

    /* save the old attrs */
    Symbol **old_symbols = symtable->symbols;
    uint32_t old_bscale = symtable->bscale;

    /* allocate new entry array and set bucket count */
    symtable->symbols = calloc(NBUCKETS[bucketscale],
            sizeof(*(symtable->symbols)));
    if (!symtable->symbols)
        die("Error allocating new, larger symtable entry array\n");
    symtable->bscale = bucketscale;

    Symbol *cur = NULL, *prev = NULL;
    Symbol *inserted = NULL;
    int i = 0;
    for (i = 0; i < NBUCKETS[old_bscale]; i ++) {
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
SymbolTable *symtable_new(uint32_t bucketscale)
{
    SymbolTable *symtable = calloc(1, sizeof(*symtable));
    if (!symtable)
        die("Error allocating symbol table\n");
    LUCI_DEBUG("%s\n", "Allocated symbol table");

    if (bucketscale < 0 || bucketscale >= N_BUCKET_OPTIONS)
        die("Symbol Table scale out of bounds\n");
    symtable->bscale = bucketscale;

    symtable->symbols = calloc(NBUCKETS[bucketscale],
            sizeof(*(symtable->symbols)));
    if (!symtable->symbols)
        die("Error allocating symtable symbols array\n");
    LUCI_DEBUG("%s\n", "Allocated symbol table symbols array");

    /* make Symbol Table object array size = bucket count / 4 */
    symtable->size = NBUCKETS[bucketscale] >> 2;
    symtable->objects = calloc(symtable->size, sizeof(*(symtable->objects)));
    if (!symtable->objects)
        die("Error allocating symtable objects array\n");

    /* Set bit which identifies that this table owns the objects array */
    symtable->owns_objects = 1;

    LUCI_DEBUG("%s\n", "Allocated symbol table objects array");

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
    for (i = 0; i < NBUCKETS[symtable->bscale]; i ++) {
        cur = symtable->symbols[i];
        while (cur) {
            prev = cur;
            cur = cur->next;
            symbol_delete(prev);
        }
    }

    /* only deallocate objects if the table still owns the array */
    if (symtable->owns_objects > 0) {
        /* deallocate all objects in table */
        for (i = 0; i < symtable->count; i ++) {
            if (symtable->objects[i]) {
                decref(symtable->objects[i]);
            }
        }
        free(symtable->objects);
        symtable->objects = NULL;
    }

    free(symtable->symbols);
    symtable->symbols = NULL;
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
 * Returns the ID of the symbol
 * if SYMCREATE flag is passed, a new symbol
 * will be created if it doesn't already exist
 */
int symtable_id(SymbolTable *symtable, const char *name, uint8_t flags)
{
    Symbol *sym = NULL;

    if (!symtable) {
        die("Symbol Table not allocated\n");
    }

    /* Try to find symbol in table */
    sym = find_symbol_by_name(symtable, name);
    /* if found, return it's index (ID) */
    if (sym) {
        return sym->index;
    } else if (!(flags & SYMCREATE)) {
        /* don't create symbol, return -1 */
        return -1;
    }
    /* otherwise, we need to create the symbol and insert it */

    /* Increase count of Symbol/Object pairs */
    if (symtable->count >= symtable->size) {
        /* ensure object array is large enough */
        symtable->size <<= 1;
        symtable->objects = realloc(symtable->objects,
                symtable->size * sizeof(*(symtable->objects)));
        if (!symtable->objects) {
            die("Could not increase symbol table object array size\n");
        }
        int i;
        for (i = symtable->count; i < symtable->size; i++) {
            symtable->objects[i] = NULL;
        }
    }
    /* create the new symbol */
    sym = symbol_new(name, symtable->count);

    /* insert the new symbol */
    symtable_insert(symtable, sym);

    /* return symbol's ID (object array index) */
    return symtable->count++;
}

/**
 * Replaces the object in the table pertaining to id
 * Returns old object if it exists
 */
void symtable_set(SymbolTable *symtable, LuciObject *obj, uint32_t id)
{
    LuciObject *old = NULL;

    if ((id < 0) || (id >= symtable->count))
        die("Symbol id out of bounds\n");

    /* Decref any existing object */
    if (symtable->objects[id])
        decref(symtable->objects[id]);
    symtable->objects[id] = obj;
    incref(obj);
}

LuciObject **symtable_get_objects(SymbolTable *symtable)
{
    if (!symtable) {
        return NULL;
    }

    symtable->owns_objects = 0;

    return symtable->objects;
}

