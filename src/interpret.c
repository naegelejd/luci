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


    Stack lstack, lframe;
    st_init(&lstack);
    st_init(&lframe);

    int ip = 0;
    int i = 0;

    for(EVER) {
        instr = prog->instructions[ip++];
        if (instr >> 11 >= JUMP) {
            /* read next instruction to build jmp address */
            a = ((0x7FF & instr) << 16) + prog->instructions[ip++];
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

            case LOADK:
                yak("LOADK %d\n", a);
                x = cotable_get(prog->cotable, a);
                st_push(&lstack, x);
                break;

            case LOADS:
                yak("LOADS %d\n", a);
                x = symtable_get(prog->symtable, a);
                st_push(&lstack, x);
                break;

            case DUP:
                yak("DUP\n");
                /* duplicate object on top of stack
                 * and push it back on */
                x = copy_object(st_top(&lstack));
                st_push(&lstack, x);
                break;

            case STORE:
                yak("STORE %d\n", a);
                /* pop object off of stack */
                x = st_pop(&lstack);
                symtable_set(prog->symtable, x, a);
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
                x = st_pop(&lstack);    /* funcptr obj */

                for (i = 0; i < a; i++) {
                    y = st_pop(&lstack);
                    st_push(&lframe, y);
                }
                /* call func, passing frame and arg count */
                z = x->value.func(&lframe, a);
                st_push(&lstack, z);    /* always push return val */
                /* flush stack frame */

                while (!st_empty(&lframe)) {
                    y = st_pop(&lframe);
                    destroy(y);
                }

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
                    die("Invalid type in list access\n");
                }

                z = list_get_object(x, y->value.i_val);

                destroy(x);
                destroy(y);
                st_push(&lstack, z);
                break;

            case LISTPUT:
                yak("LISTPUT %d\n", a);
                /* pop list */
                z = st_pop(&lstack);
                /* pop index */
                y = st_pop(&lstack);
                /* pop right hand value */
                x = st_pop(&lstack);

                if (y->type != obj_int_t) {
                    die("Invalid type in list assign\n");
                }
                i = y->value.i_val;
                destroy(y);
                y = list_set_object(z, x, i);
                /* y is the old object */
                destroy(y);
                break;

            case JUMP:
                yak("JUMP %X\n", a);
                ip = a;
                break;

            case JUMPZ:
                yak("JUMPZ %X\n", a);
                x = st_pop(&lstack);
                if (x->value.i_val == 0)
                    ip = a;
                destroy(x);
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