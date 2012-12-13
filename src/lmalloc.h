/*
 * See Copyright Notice in luci.h
 */

#ifndef LUCI_MALLOC_H
#define LUCI_MALLOC_H

#include <stdlib.h>


struct lmalloc_arena {
    unsigned int bytes;
    void * ptr;
};

struct chunk_hdr {
    void *prev;
    void *next;
    size_t size;
};


void lmalloc_init();
void lmalloc_finalize();
void * lmalloc(size_t size);
void lfree(void *);
void lmalloc_print();


#endif /* LUCI_MALLOC_H */
