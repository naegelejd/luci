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
#include "map.h"
#include "interpret.h"
#include "stack.h"
#include "compile.h"
#include "binop.h"


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
    register LuciObject *x = NULL;
    register LuciObject *y = NULL;
    register LuciObject *z = NULL;
    //Instruction instr = 0;
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
            /* store the new object */
            frame->locals[a] = x;
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
            if (TYPEOF(x) == obj_func_t) {
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
                    z = copy_object(y);
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
            }
            else {
                DIE("%s", "Can't call something that isn't a function\n");
            }
        }
        DISPATCH(NEXT_OPCODE);

        HANDLE(RETURN)
            LUCI_DEBUG("%s\n", "RETURN");
            /* delete previously active frame (and all its locals) */
            Frame_delete_copy(frame);
            /* pop function stack frame and replace active frame */
            frame = st_pop(&framestack);
            /* restore saved instruction pointer */
            ip = frame->ip;
        DISPATCH(NEXT_OPCODE);

        HANDLE(MKMAP)
            LUCI_DEBUG("MKMAP %d\n", a);
            x = LuciMap_new();
            for (i = 0; i < a; i ++) {
                /* first item is the value */
                y = st_pop(&lstack);
                /* then the key */
                z = st_pop(&lstack);
                /* add the key & value to the map */
                map_set(x, z, y);
            }
            if (!x) {
                DIE("%s\n", "a horrible flaming NULL map death");
            }
            st_push(&lstack, x);
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

        HANDLE(CGET)
        {
            LUCI_DEBUG("%s\n", "CGET");
            /* pop container */
            x = st_pop(&lstack);

            /* determine if container is list or map */
            switch (TYPEOF(x)) {

            case obj_list_t:
            {
                /* pop index */
                y = st_pop(&lstack);
                if (y->type != obj_int_t) {
                    DIE("%s", "Invalid list index type\n");
                }

                /* get a copy of the obj in list at index */
                z = list_get_object(x, ((LuciIntObj *)y)->i);
                st_push(&lstack, z);
            } break;

            case obj_map_t:
            {
                /* pop key */
                y = st_pop(&lstack);
                /* get the val for key 'y' */
                z = map_get(x, y);
                st_push(&lstack, z);
            } break;

            default:
                DIE("%s\n", "Cannot store value in a non-container object");
            }
        }
        DISPATCH(NEXT_OPCODE);

        HANDLE(CPUT)
        {
            LUCI_DEBUG("%s\n", "CPUT");
            /* pop container */
            x = st_pop(&lstack);

            /* determine if container is list or map */
            switch (TYPEOF(x)) {

            case obj_list_t:
            {
                /* pop index */
                y = st_pop(&lstack);
                /* pop right hand value */
                z = st_pop(&lstack);

                if (y->type != obj_int_t) {
                    DIE("%s", "Invalid type in list assign\n");
                }
                /* re-use 'y' to obtain a pointer to the old object at index i */
                y = list_set_object(x, z, ((LuciIntObj *)y)->i);
            } break;

            case obj_map_t:
            {
                /* pop key */
                y = st_pop(&lstack);
                /* pop right hand value */
                z = st_pop(&lstack);
                /* re-use 'y' to obtain a pointer to the old val */
                y = map_set(x, y, z);
            } break;

            default:
                DIE("%s\n", "Cannot store value in a non-container object");
            }
        }
        DISPATCH(NEXT_OPCODE);

        HANDLE(MKITER)
            LUCI_DEBUG("%s\n", "MKITER");
            /* y should be a container */
            y = st_pop(&lstack);
            x = LuciIterator_new(y, 1); /* step = 1 */
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
