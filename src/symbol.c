/*
 * See Copyright Notice in luci.h
 */

/**
 * @file symbol.c
 */

#include "luci.h"
#include "symbol.h"
#include "lucitypes.h"

static Symbol *symbol_new(const char *, unsigned int);
static void symbol_delete(Symbol *);

static SymbolTable *symtable_insert(SymbolTable *, Symbol *);
static SymbolTable *symtable_resize(SymbolTable *, unsigned int);
static Symbol *find_symbol_by_name(SymbolTable *, const char *);

static unsigned int hash_symbol(SymbolTable *, const char *);


enum { N_BUCKET_OPTIONS = 26 };
/**
 * Array of prime number hash table sizes.
 *
 * Numbers from:
 * http://planetmath.org/GoodHashTablePrimes.html
 */
static unsigned int NBUCKETS[N_BUCKET_OPTIONS] = {
    97, 193, 389, 769, 1543, 3079, 6151, 12289,
    24593, 49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189,
    805306457, 1610612741, 0
};

/**
 * djb2 algorithm (Dave Jenkins)
 *
 * @param symtable pointer to symbol table
 * @param name string to hash
 * @return hash of name
 */
static unsigned int hash_symbol(SymbolTable *symtable, const char *name)
{
    unsigned int h = 5381;
    int c;

    while ((c = *name++)) {
        h = ((h << 5) + h) + c;
    }
    return h % NBUCKETS[symtable->bscale];
}

/**
 * Allocates a new symbol
 *
 * @param name name of new symbol
 * @param index index of corresponding LuciObject in SymbolTable array
 * @return new Symbol
 */
static Symbol *symbol_new(const char *name, unsigned int index)
{
    Symbol *new = alloc(sizeof(*new));
    new->name = strdup(name);   /* copy the symbol string */
    new->index = index;
    new->next = NULL;
    return new;
}

/**
 * Deletes an allocated symbol
 *
 * @param del symbol to delete
 */
static void symbol_delete(Symbol *del)
{
    if (del) {
        if (del->name) {
            free(del->name);
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
 *
 * @param symtable pointer to SymbolTable
 * @param new_symbol pointer to new Symbol
 * @return symtable
 */
static SymbolTable *symtable_insert(SymbolTable *symtable, Symbol *new_symbol)
{
    Symbol *cur = NULL, *prev = NULL;

    /* resize symbol table if if is over half full */
    if (symtable->count > (NBUCKETS[symtable->bscale] >> 1)) {
        symtable = symtable_resize(symtable, symtable->bscale++);
    }

    /* calculate hash of the symbol's name */
    unsigned int hash = hash_symbol(symtable, new_symbol->name);

    cur = symtable->symbols[hash];

    /* if bucket is empty, stuff new symbol into it */
    if (!cur) {
        symtable->symbols[hash] = new_symbol;
        return symtable;
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

/**
 * Resizes the SymbolTable, re-hashing all existing symbols
 *
 * @param symtable pointer to symbol table
 * @param bucketscale index into array of possible table sizes (primes)
 * @returns resized symtable
 */
static SymbolTable *symtable_resize(SymbolTable *symtable, unsigned int bucketscale)
{
    /* no shrink implementation defined */
    if (symtable->bscale >= bucketscale)
        return symtable;

    LUCI_DEBUG("Resizing symtable from %d to %d\n",
            NBUCKETS[symtable->bscale], NBUCKETS[bucketscale]);

    /* save the old attrs */
    Symbol **old_symbols = symtable->symbols;
    unsigned int old_bscale = symtable->bscale;

    /* allocate new entry array and set bucket count */
    symtable->symbols = alloc(NBUCKETS[bucketscale] *
            sizeof(*(symtable->symbols)));
    if (!symtable->symbols)
        DIE("%s", "Error allocating new, larger symtable entry array\n");
    symtable->bscale = bucketscale;

    Symbol *cur = NULL;
    int i = 0;
    for (i = 0; i < NBUCKETS[old_bscale]; i ++) {
        cur = old_symbols[i];
        while (cur) {
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
 *
 * @param bucketscale index into array of possible table sizes (primes)
 * @return pointer to new SymbolTable
 */
SymbolTable *symtable_new(unsigned int bucketscale)
{
    SymbolTable *symtable = alloc(sizeof(*symtable));
    LUCI_DEBUG("%s\n", "Allocated symbol table");

    if (bucketscale >= N_BUCKET_OPTIONS) {
        DIE("%s", "Symbol Table scale out of bounds\n");
    }
    symtable->bscale = bucketscale;

    symtable->symbols = alloc(NBUCKETS[bucketscale] *
            sizeof(*(symtable->symbols)));
    LUCI_DEBUG("%s\n", "Allocated symbol table symbols array");

    /* make Symbol Table object array size = bucket count / 4 */
    symtable->size = NBUCKETS[bucketscale] >> 2;
    symtable->objects = alloc(symtable->size * sizeof(*(symtable->objects)));
    LUCI_DEBUG("%s\n", "Allocated symbol table objects array");

    symtable->owns_objects = true;

    return symtable;
}

/**
 * Deallocates all memory allocated for Symbol Table.
 *
 * Should be used at the end of interpretation,
 * as all Objects referenced by symbols will also
 * be destroyed.
 *
 * @param symtable pointer to symbol table
 */
void symtable_delete(SymbolTable *symtable)
{
    Symbol *cur = NULL, *prev = NULL;
    int i;
    /* deallocate all symbols in table */
    for (i = 0; i < NBUCKETS[symtable->bscale]; i++) {
        cur = symtable->symbols[i];
        while (cur) {
            prev = cur;
            cur = cur->next;
            symbol_delete(prev);
        }
    }

    if (symtable->owns_objects) {
        free(symtable->objects);
        symtable->objects = NULL;
    }

    free(symtable->symbols);
    symtable->symbols = NULL;
    free(symtable);

    return;
}

/**
 * Searches the symbol table for a specific symbol name
 *
 * @param symtable pointer to symbol table
 * @param name symbol name to search for
 * @return matching symbol or NULL
 */
static Symbol *find_symbol_by_name(SymbolTable *symtable, const char *name)
{
    Symbol *cur = NULL;
    /* Compute hash and bound it by the table's # of buckets */
    unsigned int hash = hash_symbol(symtable, name);

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
 *
 * If SYMCREATE flag is passed, a new symbol
 * will be created if it doesn't already exist
 *
 * @param symtable pointer to symbol table
 * @param name symbol name
 * @param flags bitmask defining symbol creation options
 * @return id an integer id for the symbol or -1 on failure
 */
int symtable_id(SymbolTable *symtable, const char *name, symtable_flags flags)
{
    Symbol *sym = NULL;

    if (!symtable) {
        DIE("%s", "Symbol Table not allocated\n");
    } else if (symtable->bscale >= N_BUCKET_OPTIONS) {
        DIE("Symtable full, cannot add symbol %s\n", name);
    }

    /* Try to find symbol in table */
    sym = find_symbol_by_name(symtable, name);

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
            DIE("%s", "Could not increase symbol table object array size\n");
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
 * Replaces the object in the table at index `id`
 *
 * @param symtable symtable pointer
 * @param obj LuciObject to set as value at index `id`
 * @param id index in the table at which to set a new object value
 * @return object that previously resided at index `id`
 */
void symtable_set(SymbolTable *symtable, LuciObject *obj, unsigned int id)
{
    if (id >= symtable->count)
        DIE("%s", "Symbol id out of bounds\n");

    symtable->objects[id] = obj;
}

/**
 * Returns a copy of the symbol table's array of objects
 *
 * @param symtable pointer to symbol table
 * @return copy of array of LuciObjects
 */
LuciObject **symtable_copy_objects(SymbolTable *symtable)
{
    if (!symtable) {
        DIE("%s\n", "Cannot copy object array from NULL SymbolTable\n");
    }

    size_t bytes = symtable->count * sizeof(*symtable->objects);
    LuciObject **copy = alloc(bytes);
    memcpy(copy, symtable->objects, bytes);

    return copy;
}

/**
 * Returns the symbol table's array of objects
 *
 * Calling this causes the symtable to NOT delete its objects when
 * itself freed.
 *
 * @param symtable pointer to symbol table
 * @return array of LuciObjects
 */
LuciObject **symtable_give_objects(SymbolTable *symtable)
{
    if (!symtable) {
        DIE("%s\n", "Cannot copy object array from NULL SymbolTable\n");
    }

    symtable->owns_objects = false;

    return symtable->objects;
}
