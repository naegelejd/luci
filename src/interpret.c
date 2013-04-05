/*
 * See Copyright Notice in luci.h
 */

/**
 * @file interpret.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "luci.h"
#include "object.h"
#include "interpret.h"
#include "stack.h"
#include "compile.h"


/**
 * Main interpreter loop
 *
 * @param frame top-level Frame to being interpreting
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

#include "dispatch.h"  /* include static jump table */

#define INTERP_INIT()   DISPATCH
#define SWITCH
#define DEFAULT
#define HANDLE(op)      do_##op: { a = GETARG; }
#define DISPATCH        goto *dispatch_table[GETOPCODE]

#else /* __GNUC__ */

#define INTERP_INIT()
#define SWITCH          switch(GETOPCODE)
#define DEFAULT         default: goto done_eval;
#define HANDLE(op)      case (op): { a = GETARG; }
#define DISPATCH        break

#endif /* __GNUC__ */
/**************************************************************/

#define FETCH(c)        (ip += (c))
#define READ            (*ip)
#define GETOPCODE       OPCODE(READ)
#define GETARG          OPARG(READ)

    Stack lstack, framestack;
    st_init(&lstack);
    st_init(&framestack);

    LuciObject* lfargs[MAX_LIBFUNC_ARGS];
    register LuciObject *x = LuciNilObj;
    register LuciObject *y = LuciNilObj;
    register LuciObject *z = LuciNilObj;
    int a;
    int i = 0;
    Instruction *ip = frame->instructions;

    for (i = 0; i < MAX_LIBFUNC_ARGS; i++) {
        lfargs[i] = LuciNilObj;
    }
    i = 0;

    INTERP_INIT();

    /* Begin interpreting instructions */
#define EVER ;; /* saw this on stackoverflow. so dumb */
    for(EVER) {
        SWITCH {

        HANDLE(NOP) {
            LUCI_DEBUG("%s\n", "NOP");
        }
        FETCH(1);
        DISPATCH;

        HANDLE(ADD) {
            LUCI_DEBUG("%s\n", "ADD");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->add(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(SUB) {
            LUCI_DEBUG("%s\n", "SUB");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->sub(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(MUL) {
            LUCI_DEBUG("%s\n", "MUL");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->mul(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(DIV) {
            LUCI_DEBUG("%s\n", "DIV");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->div(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(MOD) {
            LUCI_DEBUG("%s\n", "MOD");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->mod(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(POW) {
            LUCI_DEBUG("%s\n", "POW");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->pow(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(EQ) {
            LUCI_DEBUG("%s\n", "EQ");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->eq(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(NEQ) {
            LUCI_DEBUG("%s\n", "NEQ");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->neq(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LT) {
            LUCI_DEBUG("%s\n", "LT");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->lt(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(GT) {
            LUCI_DEBUG("%s\n", "GT");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->gt(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LTE) {
            LUCI_DEBUG("%s\n", "LTE");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->lte(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(GTE) {
            LUCI_DEBUG("%s\n", "GTE");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->gte(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LGOR) {
            LUCI_DEBUG("%s\n", "LGOR");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->lgor(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LGAND) {
            LUCI_DEBUG("%s\n", "LGAND");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->lgand(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(BWXOR) {
            LUCI_DEBUG("%s\n", "BWXOR");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->bwxor(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(BWOR) {
            LUCI_DEBUG("%s\n", "BWOR");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->bwor(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(BWAND) {
            LUCI_DEBUG("%s\n", "BWAND");
            y = st_pop(&lstack);
            x = st_pop(&lstack);
            z = x->type->bwand(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(NEG) {
            LUCI_DEBUG("%s\n", "NEG");
            x = st_pop(&lstack);
            y = x->type->neg(x);
            st_push(&lstack, y);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LGNOT) {
            LUCI_DEBUG("%s\n", "LGNOT");
            x = st_pop(&lstack);
            y = x->type->lgnot(x);
            st_push(&lstack, y);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(BWNOT) {
            LUCI_DEBUG("%s\n", "BWNOT");
            x = st_pop(&lstack);
            y = x->type->bwnot(x);
            st_push(&lstack, y);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(POP)
        {
            LUCI_DEBUG("%s\n", "POP");
            x = st_pop(&lstack);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(PUSHNIL)
            LUCI_DEBUG("%s\n", "PUSHNIL");
            st_push(&lstack, LuciNilObj);
        FETCH(1);
        DISPATCH;

        HANDLE(LOADK)
            LUCI_DEBUG("LOADK %d\n", a);
            st_push(&lstack, frame->constants[a]);
        FETCH(1);
        DISPATCH;

        HANDLE(LOADS)
            LUCI_DEBUG("LOADS %d\n", a);
            x = frame->locals[a];
            st_push(&lstack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(LOADG)
            LUCI_DEBUG("LOADG %d\n", a);
            x = frame->globals[a];
            st_push(&lstack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(LOADB)
            LUCI_DEBUG("LOADB %d\n", a);
            x = builtins[a];
            st_push(&lstack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(DUP)
            LUCI_DEBUG("%s\n", "DUP");
            /* duplicate object on top of stack
             * and push it back on */
            x = st_peek(&lstack);
            y = x->type->copy(x);
            st_push(&lstack, y);
        FETCH(1);
        DISPATCH;

        HANDLE(STORE)
            LUCI_DEBUG("STORE %d\n", a);
            /* pop object off of stack */
            x = st_pop(&lstack);
            /* store the new object */
            if (x->type->shallow) {
                frame->locals[a] = x;
            } else {
                frame->locals[a] = x->type->copy(x);
            }
        FETCH(1);
        DISPATCH;

        HANDLE(CALL)
        {
            LUCI_DEBUG("CALL %d\n", a);
            x = st_pop(&lstack);    /* function object */

            /* setup user-defined function */
            if (ISTYPE(x, obj_func_t)) {
                /* save instruction pointer */
                frame->ip = ip;

                /* push a func frame object onto framestack */
                st_push(&framestack, frame);

                /* activate a copy of the function frame */
                frame = Frame_copy(((LuciFunctionObj *)x)->frame);

                /* check that the # of arguments equals the # of parameters */
                if (a < frame->nparams) {
                    DIE("%s", "Missing arguments to function.\n");
                } else if (a > frame->nparams) {
                    DIE("%s", "Too many arguments to function.\n");
                }

                /* pop arguments and push COPIES into locals */
                for (i = 0; i < a; i++) {
                    y = st_pop(&lstack);
                    if (y->type->shallow) {
                        frame->locals[i] = y;
                    } else {
                        frame->locals[i] = y->type->copy(y);
                    }
                }

                /* reset instruction pointer and carry on our merry way */
                /* NOTE: while ugly, we decrement ip by one instruction
                 * so that the following FETCH call starts at the 1st
                 * instruction!!! */
                ip = frame->instructions - 1;
            }

            /* call library function */
            else if (ISTYPE(x, obj_libfunc_t)) {
                if (a >= MAX_LIBFUNC_ARGS) {
                    DIE("%s\n", "Too many arguments to function");
                } else if (a < AS_LIBFUNC(x)->min_args) {
                    DIE("%s\n", "Missing arguments to function");
                }

                /* pop args and push into args array */
                /* must happen in reverse */
                for (i = a - 1; i >= 0; i--) {
                    y = st_pop(&lstack);
                    if (y->type->shallow) {
                        lfargs[i] = y;
                    } else {
                        lfargs[i] = y->type->copy(y);
                    }
                }

                /* call func, passing args array and arg count */
                z = ((LuciLibFuncObj *)x)->func(lfargs, a);
                st_push(&lstack, z);    /* always push return val */
            }
            else {
                DIE("%s", "Can't call something that isn't a function\n");
            }
        }
        FETCH(1);
        DISPATCH;

        HANDLE(RETURN)
            LUCI_DEBUG("%s\n", "RETURN");
            /* delete previously active frame (and all its locals) */
            Frame_delete_copy(frame);
            /* pop function stack frame and replace active frame */
            frame = st_pop(&framestack);
            /* restore saved instruction pointer */
            ip = frame->ip;
        FETCH(1);
        DISPATCH;

        HANDLE(MKMAP)
            LUCI_DEBUG("MKMAP %d\n", a);
            x = LuciMap_new();
            for (i = 0; i < a; i ++) {
                /* first item is the value */
                y = st_pop(&lstack);
                /* then the key */
                z = st_pop(&lstack);
                /* add the key & value to the map */
                x->type->cput(x, z, y);
            }
            st_push(&lstack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(MKLIST)
            LUCI_DEBUG("MKLIST %d\n", a);
            x = LuciList_new();
            for (i = 0; i < a; i ++) {
                y = st_pop(&lstack);
                /* LuciList_append */
                x->type->lt(x, y);
            }
            st_push(&lstack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(CGET)
        {
            LUCI_DEBUG("%s\n", "CGET");
            /* pop container */
            x = st_pop(&lstack);
            /* pop 'index' */
            y = st_pop(&lstack);
            /* cget from the container */
            z = x->type->cget(x, y);
            st_push(&lstack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(CPUT)
        {
            LUCI_DEBUG("%s\n", "CPUT");
            /* pop container */
            x = st_pop(&lstack);
            /* pop index/key/... */
            y = st_pop(&lstack);
            /* pop right hand value */
            z = st_pop(&lstack);
            /* put the right hand value into the container */
            y = x->type->cput(x, y, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(MKITER)
            LUCI_DEBUG("%s\n", "MKITER");
            /* x should be a container */
            x = st_pop(&lstack);
            y = LuciIterator_new(x, 1); /* step = 1 */
            st_push(&lstack, y);
        FETCH(1);
        DISPATCH;

        HANDLE(JUMP)
            LUCI_DEBUG("JUMP %d\n", a);
        FETCH(a);
        DISPATCH;

        HANDLE(POPJUMP)
            LUCI_DEBUG("POPJUMP %d\n", a);
            x = st_pop(&lstack);
        FETCH(a);
        DISPATCH;

        HANDLE(JUMPZ)
            LUCI_DEBUG("JUMPZ %d\n", a);
            x = st_pop(&lstack);
            if (((LuciIntObj *)x)->i == 0) {
                FETCH(a);
            } else {
                FETCH(1);
            }
        DISPATCH;

        HANDLE(ITERJUMP)
            LUCI_DEBUG("ITERJUMP %d\n", a);
            x = st_peek(&lstack);
            /* get a COPY of the next object in the iterator's list */
            y = iterator_next_object(x);
            /* if the iterator returned NULL, jump to the
             * end of the for loop. Otherwise, push iterator->next */
            if (y == NULL) {
                /* pop the iterator object */
                x = st_pop(&lstack);
                FETCH(a);
            } else {
                st_push(&lstack, y);
                FETCH(1);
            }
        DISPATCH;

        HANDLE(HALT)
            LUCI_DEBUG("%s\n", "HALT");
            goto done_eval;
        DISPATCH;

        DEFAULT
                DIE("Invalid opcode: %d\n", GETOPCODE);
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
