/*
 * See Copyright Notice in luci.h
 */
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "stack.h"

#define STACK_INIT_HEIGHT   128

Stack * st_init(Stack *S)
{
    S->top = 0;
    S->size = STACK_INIT_HEIGHT;
    S->array = alloc(S->size * (sizeof(*S->array)));
    return S;
}

void st_destroy(Stack *S) {
    free(S->array);
}

void st_push(Stack *S, void *item)
{
    /* resize stack if necessary */
    if (S->top >= S->size) {
        S->size <<= 1;
        S->array = realloc(S->array, S->size * (sizeof(*S->array)));
    }
    S->array[(S->top)++] = item;
}

void* st_pop(Stack *S)
{
    return (S->array[--(S->top)]);
}

void* st_peek(Stack *S)
{
    return (S->array[S->top - 1]);
}

uint32_t st_height(Stack *S)
{
    return S->top;
}

uint8_t st_empty(Stack *S)
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
            printf("%lu  ", (unsigned long)S->array[i]);
        }
        printf("\n");
    }
}
