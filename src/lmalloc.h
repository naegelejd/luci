/*
 * See Copyright Notice in luci.h
 */

#ifndef LUCI_MALLOC_H
#define LUCI_MALLOC_H

#include <stdlib.h>

/**
 * Arena of memory for allocation
 */
struct lmalloc_arena {
    unsigned int bytes; /**< size of arena in bytes */
    void * ptr;         /**< pointer to arena */
};

/**
 * Header for a free block of memory
 */
struct chunk_hdr {
    void *prev;     /**< next free block before this */
    void *next;     /**< next free block after this */
    size_t size;    /**< size of this free block */
};


void lmalloc_init();
void lmalloc_finalize();
void * lmalloc(size_t size);
void lfree(void *);
void lmalloc_print();


#endif /* LUCI_MALLOC_H */
