/*
 * See Copyright Notice in luci.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "object.h"
#include "interpret.h"
#include "stack.h"
#include "compile.h"
#include "binop.h"

#define EVER ;;

void eval(Frame *frame)
{
    LuciObject* x = NULL, *y = NULL, *z = NULL;
    LuciObject* lfargs[MAX_LIBFUNC_ARGS];
    Instruction instr = 0;
    Stack lstack, framestack;
    st_init(&lstack);
    st_init(&framestack);

    int a;
    int ip = 0;
    int i = 0;

    for (i = 0; i < MAX_LIBFUNC_ARGS; i++) {
        lfargs[i] = NULL;
    }
    i = 0;

    /* Begin interpreting instructions */
    for(EVER) {
        instr = frame->instructions[ip++];
        if (instr >> 11 >= JUMP) {
            /* read next instruction to build jmp address */
            a = ((0x7FF & instr) << 16) + frame->instructions[ip++];
        } else {
            a = instr & 0x7FF;
        }
        switch (instr >> 11) {

            case NOP:
                yak("NOP\n");
                break;

            case POP:
                yak("POP\n");
                x = st_pop(&lstack);
                destroy(x);
                break;

            case PUSHNULL:
                yak("PUSHNULL\n");
                st_push(&lstack, NULL);
                break;

            case LOADK:
                yak("LOADK %d\n", a);
                x = copy_object(frame->constants[a]);
                st_push(&lstack, x);
                break;

            case LOADS:
                yak("LOADS %d\n", a);
                x = frame->locals[a];
                st_push(&lstack, x);
                break;

            case LOADG:
                yak("LOADG %d\n", a);
                x = frame->globals[a];
                if (x == NULL) {
                    die("Global is NULL\n");
                }
                st_push(&lstack, x);
                break;

            case DUP:
                yak("DUP\n");
                /* duplicate object on top of stack
                 * and push it back on */
                x = copy_object(st_peek(&lstack));
                st_push(&lstack, x);
                break;

            case STORE:
                yak("STORE %d\n", a);
                /* pop object off of stack */
                x = st_pop(&lstack);
                if (frame->locals[a]) {
                    decref(frame->locals[a]);
                }
                frame->locals[a] = x;
                incref(x);
                break;

            case BINOP:
                yak("BINOP %d\n", a);
                y = st_pop(&lstack);
                x = st_pop(&lstack);
                z = solve_bin_expr(x, y, a);
                destroy(x);
                destroy(y);
                st_push(&lstack, z);
                break;

            case CALL:
                yak("CALL %d\n", a);
                x = st_pop(&lstack);    /* function object */

                /* setup user-defined function */
                if (x->type == obj_func_t) {
                    /* save instruction pointer */
                    frame->ip = ip;

                    /* push a func frame object onto framestack */
                    st_push(&framestack, frame);

                    /* activate a copy of the function frame */
                    frame = Frame_copy(x->value.func.frame);

                    /* check that the # of arguments equals the # of parameters */
                    if (frame->nparams > a) {
                        die("Missing arguments to function.\n");
                    } else if (frame->nparams < a) {
                        die("Too many arguments to function.\n");
                    }

                    /* pop arguments and push COPIES into locals */
                    for (i = 0; i < a; i++) {
                        y = st_pop(&lstack);
                        z = copy_object(y);
                        incref(z);
                        frame->locals[i] = z;
                        destroy(y);
                    }

                    /* reset instruction pointer */
                    ip = 0;

                    /* carry on our merry way */
                    break;
                }

                /* call library function */
                else if (x->type == obj_libfunc_t) {
                    if (a >= MAX_LIBFUNC_ARGS) {
                        die("Too many arguments to function.\n");
                    }

                    /* pop args and push into args array */
                    for (i = 0; i < a; i++) {
                        y = st_pop(&lstack);
                        lfargs[i] = copy_object(y);
                        destroy(y);
                    }

                    /* call func, passing args array and arg count */
                    z = x->value.libfunc(lfargs, a);
                    st_push(&lstack, z);    /* always push return val */

                    /* cleanup libfunction arguments */
                    for (i = 0; i < a; i++) {
                        //printf("arg %d type = %d\n", i, lfargs[i]->type);
                        decref(lfargs[i]);
                        lfargs[i] = NULL;
                    }

                }
                else {
                    die("Can't call something that isn't a function\n");
                }

                break;

            case RETURN:
                yak("RETURN\n");

                /* if the function is returning a local variable, we need
                 * to copy it and push it back on the stack */
                x = st_pop(&lstack);
                y = copy_object(x);
                destroy(x);
                st_push(&lstack, y);

                /* delete previously active frame (and locals) */
                Frame_delete_copy(frame);
                /* pop function stack frame and replace active frame */
                frame = st_pop(&framestack);
                /* restore saved instruction pointer */
                ip = frame->ip;
                break;

            case MKLIST:
                yak("MKLIST %d\n", a);
                x = create_object(obj_list_t);
                for (i = 0; i < a; i ++) {
                    y = st_pop(&lstack);
                    list_append_object(x, y);
                }
                st_push(&lstack, x);
                break;

            case LISTGET:
                yak("LISTGET\n");
                /* pop list */
                x = st_pop(&lstack);
                /* pop index */
                y = st_pop(&lstack);
                if (y->type != obj_int_t) {
                    die("Invalid list index type\n");
                }

                /* get a copy of the obj in list at index */
                z = list_get_object(x, y->value.i);

                destroy(x);
                destroy(y);
                st_push(&lstack, z);
                break;

            case LISTPUT:
                yak("LISTPUT %d\n", a);
                /* pop list */
                x = st_pop(&lstack);
                /* pop index */
                y = st_pop(&lstack);
                /* pop right hand value */
                z = st_pop(&lstack);

                if (y->type != obj_int_t) {
                    die("Invalid type in list assign\n");
                }
                i = y->value.i;
                destroy(y);
                y = list_set_object(x, z, i);
                /* y is the old object */
                destroy(y);
                break;

            case MKITER:
                yak("MKITER\n");
                x = create_object(obj_iterator_t);
                y = st_pop(&lstack);
                /* y should be a list */
                x->value.iterator.list = y;
                st_push(&lstack, x);
                break;

            case JUMP:
                yak("JUMP %X\n", a);
                ip = a;
                break;

            case POPJUMP:
                yak("POPJUMP %X\n", a);
                x = st_pop(&lstack);
                destroy(x);
                ip = a;
                break;

            case JUMPZ:
                yak("JUMPZ %X\n", a);
                x = st_pop(&lstack);
                if (x->value.i == 0)
                    ip = a;
                destroy(x);
                break;

            case ITERJUMP:
                yak("ITERJUMP\n");
                x = st_peek(&lstack);
                /* get a COPY of the next object in the iterator's list */
                y = iterator_next_object(x);
                /* if the iterator returned NULL, jump to the
                 * end of the for loop. Otherwise, push
                 * iterator->next */
                if (y == NULL) {
                    //yak("Iterator finished\n");
                    /* pop and destroy the iterator object */
                    x = st_pop(&lstack);
                    destroy(x);
                    ip = a;
                } else {
                    st_push(&lstack, y);
                }
                break;

            case HALT:
                yak("HALT\n");
                goto done_eval;
                break;

            default:
                die("Invalid opcode: %d\n", instr >> 11);
                break;
        }
    }

done_eval:;

    return;
}
