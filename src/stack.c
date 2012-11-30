/*
 * See Copyright Notice in luci.h
 */

#include <stdio.h>
#include "stack.h"

void st_init(Stack *S)
{
    S->top = 0;
}

void st_push(Stack *S, void *obj)
{
    S->impl[(S->top)++] = obj;
}

void *st_pop(Stack *S)
{
    return (S->impl[--(S->top)]);
}

void *st_peek(Stack *S)
{
    return (S->impl[S->top - 1]);
}

int st_size(Stack *S)
{
    return S->top;
}

int st_full(Stack *S)
{
    return (S->top >= STACKMAX);
}

int st_empty(Stack *S)
{
    return (S->top <= 0);
}

void st_print(Stack *S)
{
    int i;
    if (S->top == 0)
        printf("Stack is empty.\n");
    else {
        printf("Stack contents: ");
        for (i = 0; i < S-> top; i++) {
            printf("%lu  ", (unsigned long)S->impl[i]);
        }
        printf("\n");
    }
}
