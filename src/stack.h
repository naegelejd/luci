/*
 * See Copyright Notice in luci.h
 */

/**
 * @file stack.h
 */

#ifndef STACK_H
#define STACK_H

#include "object.h"

/** A very simple, unsafe stack implementation
 * using `void *` pointers */
typedef struct stack_ {
    LuciObject **array;   /**< array of void * pointers to objects */
    unsigned int top;   /**< stack pointer */
    unsigned int size;  /**< size of allocated array */
} Stack;

Stack* st_init(Stack *);
void st_destroy(Stack *);
void st_push(Stack *, LuciObject *);
LuciObject *st_pop(Stack *);
LuciObject *st_peek(Stack *);
LuciObject *st_get(Stack *, unsigned int idx);
unsigned int st_height(Stack *);
unsigned int st_empty(Stack *);
void st_print(Stack *);

#endif
