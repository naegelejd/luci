/*
 * See Copyright Notice in luci.h
 */

/**
 * @file stack.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "luci.h"
#include "stack.h"

#define STACK_INIT_HEIGHT   128     /**< initial stack size */

/**
 * Creates and initializes the stack array
 *
 * @param S stack to initialize
 * @returns initialized stack
 */
Stack * st_init(Stack *S)
{
    S->top = 0;
    S->size = STACK_INIT_HEIGHT;
    S->array = alloc(S->size * (sizeof(*S->array)));
    return S;
}

/**
 * Frees memory allocated for the stack
 *
 * @param S stack to deallocate
 */
void st_destroy(Stack *S) {
    free(S->array);
}

/**
 * Pushes a pointer onto the stack and increment the stack pointer.
 *
 * May increase the size of the stack's array if necessary
 *
 * @param S stack to push item onto
 * @param item pointer to push onto stack
 */
void st_push(Stack *S, void *item)
{
    /* resize stack if necessary */
    if (S->top >= S->size) {
        S->size <<= 1;
        S->array = realloc(S->array, S->size * (sizeof(*S->array)));
    }
    S->array[(S->top)++] = item;
}

/**
 * Return the pointer on top of the stack and decrement the stack pointer
 *
 * @param S stack to pop pointer from
 * @returns popped pointer
 */
void* st_pop(Stack *S)
{
    return (S->array[--(S->top)]);
}

/**
 * Return the pointer on top of the stack
 *
 * @param S stack to peek from
 * @returns pointer at the top of the stack
 */
void* st_peek(Stack *S)
{
    return (S->array[S->top - 1]);
}

/**
 * Returns the pointer at the given index in the stack
 *
 * @param S stack to get from
 * @param idx index in stack
 * @returns pointer at index
 */
void* st_get(Stack *S, uint32_t idx)
{
    if (idx > (S->top - 1)) {
        return NULL;
    }
    return S->array[idx];
}

/**
 * Return the stack's height
 *
 * @param S stack to return height of
 * @returns height of stack
 */
uint32_t st_height(Stack *S)
{
    return S->top;
}

/**
 * Return whether the stack is empty
 *
 * @param S stack to check for emptiness
 * @returns non-zero if empty, 0 if not empty
 */
uint8_t st_empty(Stack *S)
{
    return (S->top <= 0);
}

/**
 * Prints the stack. Not pretty since it's just pointers.
 *
 * @param S stack to 'print'
 */
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
