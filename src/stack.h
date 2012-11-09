/*
 * See Copyright Notice in luci.h
 */

#ifndef STACK_H
#define STACK_H

/* include LuciObject definition */
#include "object.h"

#define STACKMAX    64

typedef struct _Stack {
    LuciObject *impl[STACKMAX];
    int top;
} Stack;

void st_init(Stack *);
void st_push(Stack *, LuciObject *);
LuciObject *st_pop(Stack *);
LuciObject *st_top(Stack *);
int st_size(Stack *);
int st_full(Stack *);
int st_empty(Stack *);
void st_print(Stack *);

#endif
