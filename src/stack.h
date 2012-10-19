#ifndef STACK_H
#define STACK_H

/* include LuciObject definition */
#include "common.h"

#define STACKMAX    64

typedef struct {
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
