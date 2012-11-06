/*
 * See Copyright Notice in luci.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "object.h"
#include "interpret.h"
#include "symbol.h"
#include "constant.h"
#include "stack.h"
#include "compile.h"
#include "functions.h"

#define EVER ;;

void eval(Program *prog)
{
    LuciObject *x = NULL, *y = NULL, *z = NULL;
    Instruction instr = 0;
    int a;


    Stack lstack;
    st_init(&lstack);

    int ip = 0;
    int i = 0;

    for(EVER) {
        instr = prog->instructions[ip++];
        a = instr & 0x7FF;
        switch (instr >> 11) {

            case NOP:
                break;

            case LOADK:
                x = cotable_get(prog->cotable, a);
                st_push(&lstack, x);
                break;

            case LOADS:
                x = symtable_get(prog->symtable, a);
                /* push object onto stack */
                st_push(&lstack, x);
                break;

            case DUP:
                /* duplicate object on top of stack
                 * and push it back on */
                x = copy_object(st_top(&lstack));
                st_push(&lstack, x);
                break;

            case STORE:
                /* pop object off of stack */
                x = st_pop(&lstack);
                symtable_set(prog->symtable, x, a);
                break;

            case BINOP:
                y = st_pop(&lstack);
                x = st_pop(&lstack);
                z = solve_bin_expr(x, y, a);
                st_push(&lstack, z);
                break;

            case CALL:
                x = st_pop(&lstack);    /* funcptr obj */
                y = st_pop(&lstack);    /* arglist obj */
                z = x->value.func(y);
                if (z) st_push(&lstack, z);
                break;

            case MKLIST:
                x = create_object(obj_list_t);
                for (i = 0; i < a; i ++)
                    list_append_object(x, st_pop(&lstack));
                st_push(&lstack, x);
                break;

            case JUMP:
                ip = a;
                break;

            case JUMPZ:
                x = st_pop(&lstack);
                if (x->value.i_val == 0)
                    ip = a;
                break;

            case EXIT:
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
