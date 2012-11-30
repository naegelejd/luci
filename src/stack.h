/*
 * See Copyright Notice in luci.h
 */

#ifndef STACK_H
#define STACK_H

#define STACKMAX    64

typedef struct _Stack {
    void *impl[STACKMAX];
    int top;
} Stack;

void st_init(Stack *);
void st_push(Stack *, void *);
void *st_pop(Stack *);
void *st_peek(Stack *);
int st_size(Stack *);
int st_full(Stack *);
int st_empty(Stack *);
void st_print(Stack *);

#endif
