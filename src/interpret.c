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


/**
 * Main interpreter loop
 */
void eval(Frame *frame)
{

/******** Computed Goto Definitions (Labels as Values) ********/

/* Popular interpreters such as Python and Ruby utilize this
 * GCC extension for up to 20% performance improvement.
 *
 * The advantage of using of an explicit jump table and explicit
 * indirect jump instruction after each opcode's execution is
 * described in the Python 3.3 source (ceval.c).
 *
 * Eli Bendersky also has a good explanation and demo available
 * at http://eli.thegreenplace.net/2012/07/12/computed-goto-for-efficient-dispatch-tables
 *
 * NOTE (directly from Python 3.3):
 * "care must be taken that the compiler doesn't try to 'optimize' the
 * indirect jumps by sharing them between all opcodes. Such optimizations
 * can be disabled on gcc by using the -fno-gcse flag (or possibly
 * -fno-crossjumping)."
 */

/* Using __GNUC__ for now, which is always defined by GCC */
#ifdef __GNUC__

#include "dispatch.h";  /* include static jump table */

#define INTERP_INIT()   DISPATCH(NEXT_OPCODE)
#define SWITCH(instr)
#define DEFAULT
#define HANDLE(op)      do_##op: { a = OPARG; }
//#define DISPATCH(instr) goto *dispatch_table[OPCODE(NEXT)]
#define DISPATCH(instr) goto *dispatch_table[instr]

#else /* __GNUC__ */

#define INTERP_INIT()
//#define SWITCH(instr)   switch(OPCODE(NEXT)) {
#define SWITCH(instr)   switch(instr)
#define DEFAULT         default: goto done_eval;
#define HANDLE(op)      case (op): { a = OPARG; }
#define DISPATCH(instr) break

#endif /* __GNUC__ */
/**************************************************************/

#define FETCH           frame->instructions[ip - 1]
#define NEXT_INSTR      frame->instructions[ip++]
#define NEXT_OPCODE     OPCODE(NEXT_INSTR)
#define OPCODE(i)       ((i) >> 11)
#define ISJUMP(i)       ( OPCODE(i) >= JUMP )
#define OPARG           ( ISJUMP(FETCH) ? (((0x7FF & (FETCH)) << 16) + NEXT_INSTR) : ((FETCH) & 0x7FF) )

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

    INTERP_INIT();

    /* Begin interpreting instructions */
#define EVER ;; /* saw this on stackoverflow. so dumb */
    for(EVER) {
        SWITCH(NEXT_OPCODE) {

        HANDLE(NOP) {
            LUCI_DEBUG("%s\n", "NOP");
            printf("NOP");
        }
        DISPATCH(NEXT_OPCODE);

        HANDLE(POP)
        {
            LUCI_DEBUG("%s\n", "POP");
            x = st_pop(&lstack);
        }
        DISPATCH(NEXT_OPCODE);

        HANDLE(PUSHNULL)
            LUCI_DEBUG("%s\n", "PUSHNULL");
            st_push(&lstack, NULL);
        DISPATCH(NEXT_OPCODE);

        HANDLE(LOADK)
            LUCI_DEBUG("LOADK %d\n", a);
            st_push(&lstack, frame->constants[a]);
        DISPATCH(NEXT_OPCODE);

        HANDLE(LOADS)
            LUCI_DEBUG("LOADS %d\n", a);
            x = frame->locals[a];
            st_push(&lstack, x);
        DISPATCH(NEXT_OPCODE);

        HANDLE(LOADG)
            LUCI_DEBUG("LOADG %d\n", a);
            x = frame->globals[a];
            if (x == NULL) {
                DIE("%s", "Global is NULL\n");
            }
            st_push(&lstack, x);
        DISPATCH(NEXT_OPCODE);

        HANDLE(DUP)
            LUCI_DEBUG("%s\n", "DUP");
            /* duplicate object on top of stack
             * and push it back on */
            x = copy_object(st_peek(&lstack));
            st_push(&lstack, x);
        DISPATCH(NEXT_OPCODE);

        HANDLE(STORE)
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
        DISPATCH(NEXT_OPCODE);

        HANDLE(BINOP)
            LUCI_DEBUG("BINOP %d\n", a);
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = solve_bin_expr(x, y, a);
            st_push(&lstack, z);
        DISPATCH(NEXT_OPCODE);

        HANDLE(CALL)
        {
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
                /* and carry on our merry way */
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
        }
        DISPATCH(NEXT_OPCODE);

        HANDLE(RETURN)
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
        DISPATCH(NEXT_OPCODE);

        HANDLE(MKLIST)
            LUCI_DEBUG("MKLIST %d\n", a);
            x = LuciList_new();
            for (i = 0; i < a; i ++) {
                y = st_pop(&lstack);
                list_append_object(x, y);
            }
            st_push(&lstack, x);
        DISPATCH(NEXT_OPCODE);

        HANDLE(LISTGET)
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
        DISPATCH(NEXT_OPCODE);

        HANDLE(LISTPUT)
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
        DISPATCH(NEXT_OPCODE);

        HANDLE(MKITER)
            LUCI_DEBUG("%s\n", "MKITER");
            /* y should be a list */
            y = st_pop(&lstack);
            x = LuciIterator_new(y, 1);
            st_push(&lstack, x);
        DISPATCH(NEXT_OPCODE);

        HANDLE(JUMP)
            LUCI_DEBUG("JUMP %X\n", a);
            ip = a;
        DISPATCH(NEXT_OPCODE);

        HANDLE(POPJUMP)
            LUCI_DEBUG("POPJUMP %X\n", a);
            x = st_pop(&lstack);
            ip = a;
        DISPATCH(NEXT_OPCODE);

        HANDLE(JUMPZ)
            LUCI_DEBUG("JUMPZ %X\n", a);
            x = st_pop(&lstack);
            if (((LuciIntObj *)x)->i == 0) {
                ip = a;
            }
        DISPATCH(NEXT_OPCODE);

        HANDLE(ITERJUMP)
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
        DISPATCH(NEXT_OPCODE);

        HANDLE(HALT)
            LUCI_DEBUG("%s\n", "HALT");
            goto done_eval;
        DISPATCH(NEXT_OPCODE);

        DEFAULT
        }
                //DIE("Invalid opcode: %d\n", instr >> 11);
        //}
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
