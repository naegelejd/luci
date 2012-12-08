/*
 * See Copyright Notice in luci.h
 */

#ifndef STACK_H
#define STACK_H

#include <stdint.h>

typedef struct _Stack {
    void **array;
    uint32_t top;
    uint32_t size;
} Stack;

Stack* st_init(Stack *);
void st_destroy(Stack *);
void st_push(Stack *, void *);
void *st_pop(Stack *);
void *st_peek(Stack *);
uint32_t st_height(Stack *);
uint8_t st_empty(Stack *);
void st_print(Stack *);

#endif
