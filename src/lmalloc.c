/*
 * See Copyright Notice in luci.h
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include "luci.h"
#include "lmalloc.h"


#define RELATIVE(x) ((unsigned long)(x) - (unsigned long)ARENA.ptr)

//#define LUCI_ARENA_INIT_SIZE 4096
#define LUCI_ARENA_INIT_SIZE 4194304

static struct lmalloc_arena ARENA;
static struct chunk_hdr *free_head;

static int coalesce(struct chunk_hdr *ptr);


/*
 * Allocates and zeros memory for all
 * non-LuciObject requests.
 */
void *alloc(size_t size)
{
    void *result = calloc(size, 1);
    if (!result) {
	DIE("%s", "alloc failed\n");
    }
    return result;
}


/************************************************************
 * Simple malloc implementation for fixed-size LuciObjects  *
 ************************************************************/

void lmalloc_init()
{
    LUCI_DEBUG("%s\n", "Begin initializing lmalloc");

    ARENA.ptr = calloc(LUCI_ARENA_INIT_SIZE, 1);
    if (!ARENA.ptr) {
        DIE("%s\n", "Failed to initialize lnmalloc arena.");
    }
    ARENA.bytes = LUCI_ARENA_INIT_SIZE;

    free_head = (struct chunk_hdr *)ARENA.ptr;
    LUCI_DEBUG("%s\n", "Freelist head = 0");
    free_head->prev = NULL;
    free_head->next = NULL;
    free_head->size = ARENA.bytes - sizeof(*free_head);
    LUCI_DEBUG("Freelist head size = %u\n", (uint32_t)free_head->size);
    LUCI_DEBUG("%s\n", "End initializing lmalloc");
}

void lmalloc_finalize()
{
    free(ARENA.ptr);
}

void * lmalloc(size_t size)
{
    struct chunk_hdr *hdr = NULL;
    struct chunk_hdr *prev = NULL;
    struct chunk_hdr *next = NULL;
    struct chunk_hdr *split = NULL;
    void *ret = NULL;
    size_t needed;

    LUCI_DEBUG("%s\n", "Begin lmalloc");

    /* round size to multiple of 8 for alignment */
    size = (size % 8) ? (((size / 8) + 1) * 8) : size;
    /* needed is the total chunk for user and bookkeeping */
    needed = size + sizeof(*hdr);
    LUCI_DEBUG("Size needed = %lu\n", (unsigned long)needed);

    for (hdr = free_head, prev = hdr->prev; hdr != NULL;
            prev = hdr, hdr = hdr->next)
    {
        if (hdr->size >= needed) {
            /* found a valid block */
            LUCI_DEBUG("Found valid free header @ 0x%lX\n", RELATIVE(hdr));

            /* either split the block, or give the user the whole thing
             * if splitting it wouldn't leave enough for future use.
             * i.e. if the split resulting block is <= sizeof(HEADER)
             */
            if ((hdr->size - needed) <= sizeof(*hdr)) {
                split = hdr->next;
                LUCI_DEBUG("%s", "Block too small to split\n");
            } else {
                split = (void *)hdr + needed; /* sizeof(*hdr) + size; */
                split->next = hdr->next;
                split->size = hdr->size - needed;

                /* update header size to user-requested number */
                hdr->size = size;
            }
            LUCI_DEBUG("Next free header @ 0x%lX\n", RELATIVE(split));

            split->prev = hdr->prev;
            if (prev != NULL) {
                /* update the previous header to point to the new one */
                prev->next = split;
            } else {
                /* we're splitting the head of the freelist */
                free_head = split;
            }

            /* NULL the linked-list header members of the block being returned */
            hdr->next = NULL;
            hdr->prev = NULL;

            /* calculate address to return */
            ret = (void *)hdr + sizeof(*hdr);
            LUCI_DEBUG("Returning addr 0x%lX\n", RELATIVE(ret));
            return ret;
        }
    }

    /*
    ARENA.bytes <<= 1;
    ARENA.ptr = realloc(ARENA.ptr, ARENA.bytes);
    if (!ARENA.ptr) {
        DIE("%s\n", "Failed to reallocate memory for lmalloc arena");
    }
    */

    /* no available blocks found */
    /* LUCI_DEBUG("%s\n", "Error! lmalloc returning NULL"); */
    DIE("%s", "lmalloc out of memory\n");
    return NULL;
}


void lfree(void *ptr)
{
    struct chunk_hdr *tofree = ptr - sizeof(*tofree);
    struct chunk_hdr *hdr = NULL, *prev = NULL;

    LUCI_DEBUG("Begin freeing ptr 0x%lX of size %u\n", RELATIVE(ptr),
            (uint32_t)tofree->size);

    for (hdr = free_head, prev = hdr->prev; hdr != NULL;
            prev = hdr, hdr = hdr->next)
    {
        if (hdr == tofree) {
            /* UH OH, already freed! */
            /* this can only be detected if the freed block wasn't coalesced */
            LUCI_DEBUG("OOOPS, already freed 0x%lX\n", RELATIVE(tofree));
            return;
        } else if (hdr > tofree) {
            LUCI_DEBUG("Freed block belongs before block at 0x%lX\n", RELATIVE(hdr));

            prev = hdr->prev;
            if (prev != NULL) {
                prev->next = tofree;
            } else {
                /* tofree is the new list head */
                LUCI_DEBUG("%s\n", "Freed block is now head of free list");
                free_head = tofree;
            }
            /* just insert the free list between two allocated blocks */
            tofree->prev = prev;
            tofree->next = hdr;
            /* tofree->size should be intact */

            /* backpoint the next header to block being tofree */
            hdr->prev = tofree;

            coalesce(tofree);

            return;
        }
    }
    /* At the end of the tofree list... */
    LUCI_DEBUG("Freed block @ end of free list after 0x%lX\n", RELATIVE(prev));

    tofree->next = NULL;
    tofree->prev = prev;
    /* tofree->size should be intact */
    prev->next = tofree;

    coalesce(tofree);

    return;
}

static int coalesce(struct chunk_hdr *ptr)
{
    struct chunk_hdr *prev, *next;
    int ncoalesced = 0;

    if (ptr == NULL) {
        return 0;
    }
    prev = ptr->prev;
    next = ptr->next;

    /* merge with prev if adjacent */
    if (prev && ((prev->size + sizeof(*prev) + (void *)prev) >= (void *)ptr)) {
        LUCI_DEBUG("%s\n", "Merging with previous");
        /* expand block pointed to by prev with block pointed to by ptr */
        prev->size = prev->size + ptr->size + sizeof(*ptr);
        prev->next = next;
        if (next) {
            next->prev = prev;
        }
        ncoalesced++;
    }

    /* merge with next if adjacent */
    if (next && ((ptr->size + sizeof(*ptr) + (void *)ptr) >= (void *)next)) {
        LUCI_DEBUG("%s\n", "Merging with next");
        if (ncoalesced > 0) {
            /* 'ptr' and 'prev' have already been merged into 'prev' */
            ptr = prev;
        }
        /* expand block point to by ptr */
        ptr->size += next->size + sizeof(*next);
        ptr->next = next->next;

        if (next->next) {
            /* backpoint next's next to the block we just expanded */
            next = (struct chunk_hdr *)next->next;
            next->prev = ptr;
        }
        ncoalesced++;
    }
    return ncoalesced;
}

void lmalloc_print()
{
    struct chunk_hdr *hdr = NULL;
    printf("FREE LIST:\n");
    for (hdr = free_head; hdr; hdr = hdr->next) {
        printf("    0x%lX: %u\n", RELATIVE(hdr), (uint32_t)hdr->size);
    }
}
