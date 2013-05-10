/*
 * See Copyright Notice in luci.h
 */

/**
 * @file interpret.c
 */

#include "luci.h"
#include "interpret.h"
#include "compile.h"
#include "lucitypes.h"


/**
 * Main interpreter loop
 *
 *------- Computed Goto Definitions (Labels as Values) -------
 * Popular interpreters such as Python and Ruby utilize this
 * GCC extension for up to 20% performance improvement.
 * The advantage of using of an explicit jump table and explicit
 * indirect jump instruction after each opcode's execution is
 * described in the Python 3.3 source (ceval.c).
 * Eli Bendersky also has a good explanation and demo available
 * at http://eli.thegreenplace.net/2012/07/12/computed-goto-for-efficient-dispatch-tables
 * NOTE (directly from Python 3.3):
 * "care must be taken that the compiler doesn't try to 'optimize' the
 * indirect jumps by sharing them between all opcodes. Such optimizations
 * can be disabled on gcc by using the -fno-gcse flag (or possibly
 * -fno-crossjumping)."
 *
 * @param frame top-level function to being interpreting
 */
void eval(LuciObject *frame)
{

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

    LuciObject *stack = LuciList_new();

    gc_add_root(&stack);
    gc_add_root(&frame);

    LuciObject* lfargs[MAX_LIBFUNC_ARGS];
    register LuciObject *x = LuciNilObj;
    register LuciObject *y = LuciNilObj;
    register LuciObject *z = LuciNilObj;
    int a;
    int i = 0;
    Instruction *ip = AS_FUNCTION(frame)->instructions;

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
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->add(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(SUB) {
            LUCI_DEBUG("%s\n", "SUB");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->sub(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(MUL) {
            LUCI_DEBUG("%s\n", "MUL");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->mul(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(DIV) {
            LUCI_DEBUG("%s\n", "DIV");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->div(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(MOD) {
            LUCI_DEBUG("%s\n", "MOD");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->mod(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(POW) {
            LUCI_DEBUG("%s\n", "POW");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->pow(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(EQ) {
            LUCI_DEBUG("%s\n", "EQ");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->eq(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(NEQ) {
            LUCI_DEBUG("%s\n", "NEQ");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->neq(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LT) {
            LUCI_DEBUG("%s\n", "LT");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->lt(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(GT) {
            LUCI_DEBUG("%s\n", "GT");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->gt(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LTE) {
            LUCI_DEBUG("%s\n", "LTE");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->lte(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(GTE) {
            LUCI_DEBUG("%s\n", "GTE");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->gte(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LGOR) {
            LUCI_DEBUG("%s\n", "LGOR");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->lgor(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LGAND) {
            LUCI_DEBUG("%s\n", "LGAND");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->lgand(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(BWXOR) {
            LUCI_DEBUG("%s\n", "BWXOR");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->bwxor(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(BWOR) {
            LUCI_DEBUG("%s\n", "BWOR");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->bwor(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(BWAND) {
            LUCI_DEBUG("%s\n", "BWAND");
            y = LuciList_pop(stack);
            x = LuciList_pop(stack);
            z = x->type->bwand(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(NEG) {
            LUCI_DEBUG("%s\n", "NEG");
            x = LuciList_pop(stack);
            y = x->type->neg(x);
            LuciList_push(stack, y);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(LGNOT) {
            LUCI_DEBUG("%s\n", "LGNOT");
            x = LuciList_pop(stack);
            y = x->type->lgnot(x);
            LuciList_push(stack, y);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(BWNOT) {
            LUCI_DEBUG("%s\n", "BWNOT");
            x = LuciList_pop(stack);
            y = x->type->bwnot(x);
            LuciList_push(stack, y);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(POP)
        {
            LUCI_DEBUG("%s\n", "POP");
            x = LuciList_pop(stack);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(PUSHNIL)
            LUCI_DEBUG("%s\n", "PUSHNIL");
            LuciList_push(stack, LuciNilObj);
        FETCH(1);
        DISPATCH;

        HANDLE(LOADK)
            LUCI_DEBUG("LOADK %d\n", a);
            LuciList_push(stack, AS_FUNCTION(frame)->constants[a]);
        FETCH(1);
        DISPATCH;

        HANDLE(LOADS)
            LUCI_DEBUG("LOADS %d\n", a);
            x = AS_FUNCTION(frame)->locals[a];
            LuciList_push(stack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(LOADG)
            LUCI_DEBUG("LOADG %d\n", a);
            x = AS_FUNCTION(frame)->globals[a];
            LuciList_push(stack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(LOADB)
            LUCI_DEBUG("LOADB %d\n", a);
            x = builtins[a];
            LuciList_push(stack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(DUP)
            LUCI_DEBUG("%s\n", "DUP");
            /* duplicate object on top of stack
             * and push it back on */
            x = LuciList_peek(stack);
            y = x->type->copy(x);
            LuciList_push(stack, y);
        FETCH(1);
        DISPATCH;

        HANDLE(STORE)
            LUCI_DEBUG("STORE %d\n", a);
            /* pop object off of stack */
            x = LuciList_pop(stack);
            /* store the new object */
            if (x->type->flags & FLAG_SHALLOW_COPY) {
                AS_FUNCTION(frame)->locals[a] = x;
            } else {
                AS_FUNCTION(frame)->locals[a] = x->type->copy(x);
            }
        FETCH(1);
        DISPATCH;

        HANDLE(CALL)
        {
            LUCI_DEBUG("CALL %d\n", a);
            x = LuciList_pop(stack);    /* function object */

            /* setup user-defined function */
            if (ISTYPE(x, obj_func_t)) {
                /* save instruction pointer */
                AS_FUNCTION(frame)->ip = ip;

                /* save pointer to current frame */
                LuciObject *parent_frame = frame;

                /* activate a copy of the function frame */
                frame = x->type->copy(x);

                /* check that the # of arguments equals the # of parameters */
                if (a < AS_FUNCTION(frame)->nparams) {
                    DIE("%s", "Missing arguments to function.\n");
                } else if (a > AS_FUNCTION(frame)->nparams) {
                    DIE("%s", "Too many arguments to function.\n");
                }

                /* pop arguments and push COPIES into locals */
                for (i = 0; i < a; i++) {
                    y = LuciList_pop(stack);
                    if (y->type->flags & FLAG_SHALLOW_COPY) {
                        AS_FUNCTION(frame)->locals[i] = y;
                    } else {
                        AS_FUNCTION(frame)->locals[i] = y->type->copy(y);
                    }
                }

                /* the stack is clean, now push the previous frame */
                LuciList_push(stack, parent_frame);

                /* reset instruction pointer and carry on our merry way */
                /* NOTE: while ugly, we decrement ip by one instruction
                 * so that the following FETCH call starts at the 1st
                 * instruction!!! */
                ip = AS_FUNCTION(frame)->instructions - 1;
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
                    y = LuciList_pop(stack);
                    if (y->type->flags & FLAG_SHALLOW_COPY) {
                        lfargs[i] = y;
                    } else {
                        lfargs[i] = y->type->copy(y);
                    }
                }

                gc_disable();
                /* call func, passing args array and arg count */
                z = ((LuciLibFuncObj *)x)->func(lfargs, a);
                LuciList_push(stack, z);    /* always push return val */
                gc_enable();
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
            LuciFunction_delete_copy(frame);

            /* pop the return value */
            LuciObject *return_value = LuciList_pop(stack);

            /* pop function stack frame and replace active frame */
            frame = LuciList_pop(stack);

            /* push the return value back onto the stack */
            LuciList_push(stack, return_value);

            /* restore saved instruction pointer */
            ip = AS_FUNCTION(frame)->ip;
        FETCH(1);
        DISPATCH;

        HANDLE(MKMAP)
            LUCI_DEBUG("MKMAP %d\n", a);
            x = LuciMap_new();
            for (i = 0; i < a; i ++) {
                /* first item is the value */
                y = LuciList_pop(stack);
                /* then the key */
                z = LuciList_pop(stack);
                /* add the key & value to the map */
                x->type->cput(x, z, y);
            }
            LuciList_push(stack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(MKLIST)
            LUCI_DEBUG("MKLIST %d\n", a);
            x = LuciList_new();
            for (i = 0; i < a; i ++) {
                y = LuciList_pop(stack);
                LuciList_append(x, y);
            }
            LuciList_push(stack, x);
        FETCH(1);
        DISPATCH;

        HANDLE(CGET)
        {
            LUCI_DEBUG("%s\n", "CGET");
            /* pop container */
            x = LuciList_pop(stack);
            /* pop 'index' */
            y = LuciList_pop(stack);
            /* cget from the container */
            z = x->type->cget(x, y);
            LuciList_push(stack, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(CPUT)
        {
            LUCI_DEBUG("%s\n", "CPUT");
            /* pop container */
            x = LuciList_pop(stack);
            /* pop index/key/... */
            y = LuciList_pop(stack);
            /* pop right hand value */
            z = LuciList_pop(stack);
            /* put the right hand value into the container */
            y = x->type->cput(x, y, z);
        }
        FETCH(1);
        DISPATCH;

        HANDLE(MKITER)
            LUCI_DEBUG("%s\n", "MKITER");
            /* x should be a container */
            x = LuciList_pop(stack);
            y = LuciIterator_new(x, 1); /* step = 1 */
            LuciList_push(stack, y);
        FETCH(1);
        DISPATCH;

        HANDLE(JUMP)
            LUCI_DEBUG("JUMP %d\n", a);
        FETCH(a);
        DISPATCH;

        HANDLE(POPJUMP)
            LUCI_DEBUG("POPJUMP %d\n", a);
            x = LuciList_pop(stack);
        FETCH(a);
        DISPATCH;

        HANDLE(JUMPZ)
            LUCI_DEBUG("JUMPZ %d\n", a);
            x = LuciList_pop(stack);
            if (((LuciIntObj *)x)->i == 0) {
                FETCH(a);
            } else {
                FETCH(1);
            }
        DISPATCH;

        HANDLE(ITERJUMP)
            LUCI_DEBUG("ITERJUMP %d\n", a);
            x = LuciList_peek(stack);
            /* get a COPY of the next object in the iterator's list */
            y = iterator_next_object(x);
            /* if the iterator returned NULL, jump to the
             * end of the for loop. Otherwise, push iterator->next */
            if (y == NULL) {
                /* pop the iterator object */
                x = LuciList_pop(stack);
                FETCH(a);
            } else {
                LuciList_push(stack, y);
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

    return;
}
