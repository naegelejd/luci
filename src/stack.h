/*
 * See Copyright Notice in luci.h
 */

/**
 * @file stack.h
 */

#ifndef STACK_H
#define STACK_H

#include <stdint.h>

/** A very simple, unsafe stack implementation
 * using `void *` pointers */
typedef struct _Stack {
    void **array;   /**< array of void * pointers to objects */
    uint32_t top;   /**< stack pointer */
    uint32_t size;  /**< size of allocated array */
} Stack;

Stack* st_init(Stack *);
void st_destroy(Stack *);
void st_push(Stack *, void *);
void *st_pop(Stack *);
void *st_peek(Stack *);
void *st_get(Stack *, uint32_t idx);
uint32_t st_height(Stack *);
uint8_t st_empty(Stack *);
void st_print(Stack *);

#endif
