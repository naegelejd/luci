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
    Instruction **all_instr = prog->instructions;
    Instruction *instr = NULL;
    int a, b;

    /*
    Symbol *s = NULL;
    Symbol *symtable = add_symbol(NULL, "print", sym_bfunc_t);
    symtable->data.funcptr = luci_print;
    */

    Stack lstack;
    st_init(&lstack);

    int ip = 0;
    int i = 0;

    for(EVER) {
        instr = all_instr[ip++];
        a = instr->a;
        b = instr->b;
        switch (instr->opcode) {
            case NOP:
                break;
            case LOADK:
                x = cotable_get(prog->cotable, a);
                reference_object(x);    /* increase refcount */
                st_push(&lstack, x);
                break;
            case LOADS:
                x = symtable_get(prog->symtable, a);
                /* push object onto stack */
                st_push(&lstack, x);
                break;
            case STORE:
                /* pop object off of stack */
                x = st_pop(&lstack);
                symtable_set(prog->symtable, x, a);
                break;
            case BINOP:
                x = st_pop(&lstack);
                y = st_pop(&lstack);
                /*
                printf("Performing binary op %d on top 2 stack values\n", a->v.l);
                */
                z = solve_bin_expr(x, y, a);
                printf("%d\n", z->value.i_val);
                st_push(&lstack, z);
                break;
            case CALL:
                x = st_pop(&lstack);    /* funcptr obj */
                y = st_pop(&lstack);    /* arglist obj */
                luci_print(y);
                break;
            case JUMPL:
                break;
            case JUMPN:
                break;
            case EXIT:
                puts("EXIT");
                goto done_eval;
                break;
            default:
                die("Invalid opcode: %d\n", instr->opcode);
                break;
        }
    }

done_eval:;

    return;
}
