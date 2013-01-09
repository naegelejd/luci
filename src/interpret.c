/*
 * See Copyright Notice in luci.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "luci.h"
#include "object.h"
#include "interpret.h"
#include "stack.h"
#include "compile.h"
#include "binop.h"

#ifdef __GNUC__
#define DISPATCH goto *dispatch_table[instructions[pc++]]
#else
#define DISPATCH break
#endif


#define EVER ;;

void eval(Frame *frame)
{
    Stack lstack, framestack;
    st_init(&lstack);
    st_init(&framestack);

    LuciObject* lfargs[MAX_LIBFUNC_ARGS];
    LuciObject *x = NULL;
    LuciObject *y = NULL;
    LuciObject *z = NULL;
    Instruction instr = 0;
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
                LUCI_DEBUG("%s\n", "NOP");
                break;

            case POP:
                LUCI_DEBUG("%s\n", "POP");
                x = st_pop(&lstack);
                break;

            case PUSHNULL:
                LUCI_DEBUG("%s\n", "PUSHNULL");
                st_push(&lstack, NULL);
                break;

            case LOADK:
                LUCI_DEBUG("LOADK %d\n", a);
                st_push(&lstack, frame->constants[a]);
                break;

            case LOADS:
                LUCI_DEBUG("LOADS %d\n", a);
                x = frame->locals[a];
                st_push(&lstack, x);
                break;

            case LOADG:
                LUCI_DEBUG("LOADG %d\n", a);
                x = frame->globals[a];
                if (x == NULL) {
                    DIE("%s", "Global is NULL\n");
                }
                st_push(&lstack, x);
                break;

            case DUP:
                LUCI_DEBUG("%s\n", "DUP");
                /* duplicate object on top of stack
                 * and push it back on */
                x = copy_object(st_peek(&lstack));
                st_push(&lstack, x);
                break;

            case STORE:
                LUCI_DEBUG("STORE %d\n", a);
                /* pop object off of stack */
                x = st_pop(&lstack);

                /* decref an existing object pointed to by this symbol */
                if (frame->locals[a]) {
                    decref(frame->locals[a]);
                }

                /* store and incref the new object */
                frame->locals[a] = x;
                INCREF(x);
                break;

            case BINOP:
                LUCI_DEBUG("BINOP %d\n", a);
                y = st_pop(&lstack);
                x = st_pop(&lstack);
                z = solve_bin_expr(x, y, a);
                st_push(&lstack, z);
                break;

            case CALL:
                LUCI_DEBUG("CALL %d\n", a);
                x = st_pop(&lstack);    /* function object */

                /* setup user-defined function */
                if (x->type == obj_func_t) {
                    /* save instruction pointer */
                    frame->ip = ip;

                    /* push a func frame object onto framestack */
                    st_push(&framestack, frame);

                    /* activate a copy of the function frame */
                    frame = Frame_copy(((LuciFunctionObj *)x)->frame);

                    /* check that the # of arguments equals the # of parameters */
                    if (frame->nparams > a) {
                        DIE("%s", "Missing arguments to function.\n");
                    } else if (frame->nparams < a) {
                        DIE("%s", "Too many arguments to function.\n");
                    }

                    /* pop arguments and push COPIES into locals */
                    for (i = 0; i < a; i++) {
                        y = st_pop(&lstack);
                        z = copy_object(y);
                        //INCREF(z);
                        frame->locals[i] = z;
                    }

                    /* reset instruction pointer */
                    ip = 0;

                    /* carry on our merry way */
                    break;
                }

                /* call library function */
                else if (x->type == obj_libfunc_t) {
                    if (a >= MAX_LIBFUNC_ARGS) {
                        DIE("%s", "Too many arguments to function.\n");
                    }

                    /* pop args and push into args array */
                    /* must happen in reverse */
                    for (i = a - 1; i >= 0; i--) {
                        y = st_pop(&lstack);
                        lfargs[i] = copy_object(y);
                    }

                    /* call func, passing args array and arg count */
                    z = ((LuciLibFuncObj *)x)->func(lfargs, a);
                    st_push(&lstack, z);    /* always push return val */

                    /* cleanup libfunction arguments */
                    for (i = 0; i < a; i++) {
                        //printf("arg %d type = %d\n", i, lfargs[i]->type);
                        decref(lfargs[i]);
                        lfargs[i] = NULL;
                    }

                }
                else {
                    DIE("%s", "Can't call something that isn't a function\n");
                }

                break;

            case RETURN:
                LUCI_DEBUG("%s\n", "RETURN");

                /* incref the return value at the top of the stack
                 * so it doesn't get lost(deleted) during function cleanup */
                INCREF(st_peek(&lstack));

                /* delete previously active frame (and all its locals) */
                Frame_delete_copy(frame);
                /* pop function stack frame and replace active frame */
                frame = st_pop(&framestack);
                /* restore saved instruction pointer */
                ip = frame->ip;
                break;

            case MKLIST:
                LUCI_DEBUG("MKLIST %d\n", a);
                x = LuciList_new();
                for (i = 0; i < a; i ++) {
                    y = st_pop(&lstack);
                    list_append_object(x, y);
                }
                st_push(&lstack, x);
                break;

            case LISTGET:
                LUCI_DEBUG("%s\n", "LISTGET");
                /* pop list */
                x = st_pop(&lstack);
                /* pop index */
                y = st_pop(&lstack);
                if (y->type != obj_int_t) {
                    DIE("%s", "Invalid list index type\n");
                }

                /* get a copy of the obj in list at index */
                z = list_get_object(x, ((LuciIntObj *)y)->i);
                st_push(&lstack, z);
                break;

            case LISTPUT:
                LUCI_DEBUG("LISTPUT %d\n", a);
                /* pop list */
                x = st_pop(&lstack);
                /* pop index */
                y = st_pop(&lstack);
                /* pop right hand value */
                z = st_pop(&lstack);

                if (y->type != obj_int_t) {
                    DIE("%s", "Invalid type in list assign\n");
                }
                i = ((LuciIntObj *)y)->i;

                /* re-use 'y' to obtain a pointer to the old object at index i */
                y = list_set_object(x, z, i);
                /* decref the old object from index i */
                decref(y);
                break;

            case MKITER:
                LUCI_DEBUG("%s\n", "MKITER");
                /* y should be a list */
                y = st_pop(&lstack);
                x = LuciIterator_new(y, 1);
                st_push(&lstack, x);
                break;

            case JUMP:
                LUCI_DEBUG("JUMP %X\n", a);
                ip = a;
                break;

            case POPJUMP:
                LUCI_DEBUG("POPJUMP %X\n", a);
                x = st_pop(&lstack);
                ip = a;
                break;

            case JUMPZ:
                LUCI_DEBUG("JUMPZ %X\n", a);
                x = st_pop(&lstack);
                if (((LuciIntObj *)x)->i == 0) {
                    ip = a;
                }
                break;

            case ITERJUMP:
                LUCI_DEBUG("%s\n", "ITERJUMP");
                x = st_peek(&lstack);
                /* get a COPY of the next object in the iterator's list */
                y = iterator_next_object(x);
                /* if the iterator returned NULL, jump to the
                 * end of the for loop. Otherwise, push iterator->next */
                if (y == NULL) {
                    /* pop and destroy the iterator object */
                    x = st_pop(&lstack);
                    decref(x);
                    ip = a;
                } else {
                    st_push(&lstack, y);
                }
                break;

            case HALT:
                LUCI_DEBUG("%s\n", "HALT");
                goto done_eval;
                break;

            default:
                DIE("Invalid opcode: %d\n", instr >> 11);
                break;
        }
    }

done_eval:;

    /* Since only one call stack and one object stack
     * are ever allocated, I don't think these necessarily
     * need freed at runtime-end
     */
    st_destroy(&lstack);
    st_destroy(&framestack);

    return;
}
