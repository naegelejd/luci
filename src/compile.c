#include <stdlib.h>
#include <stdio.h>
#include "compile.h"
#include "common.h"
#include "ast.h"

static void _compile(AstNode *, Program *);
static void add_instr(Program *, Opcode,
        LuciObject *, LuciObject *, LuciObject *);

/**
 * Public entry point for compilation.
 *
 * Allocates a Program struct and passes it to
 * the AST walker/compiler.
 */
void compile_ast(AstNode *root)
{
    if (!root)
        die("Nothing to compile\n");

    Program *prog = alloc(sizeof(*prog));
    prog->count = 0;
    prog->size = BASE_INSTR_COUNT;
    prog->instructions = alloc(prog->size * sizeof(*(prog->instructions)));

    /* compile the AST */
    _compile(root, prog);

    /* destroy each instruction, instruction list, and program */
    int i = 0;
    Instruction **instructions = prog->instructions;
    for (i = 0; i < prog->count; i ++) {
        free(instructions[i]);
    }
    free(prog->instructions);
    free(prog);
}

static void add_instr(Program *prog, Opcode op,
        LuciObject *a, LuciObject *b, LuciObject *c)
{
    if (!prog)
        die("Program not allocated. Can't add instruction\n");
    if (!(prog->instructions))
        die("Instruction list not allocated. Can't add instruction\n");

    /* Allocate and initialize the instruction */
    Instruction *new_instr = alloc(sizeof(*new_instr));
    new_instr->opcode = op;
    new_instr->a = a;
    new_instr->b = c;
    new_instr->b = c;

    /* Reallocate the program's instruction list if necessary */
    if (++(prog->count) > prog->size) {
        prog->size <<= 1;
        prog->instructions = realloc(prog->instructions,
                prog->size * sizeof(*(prog->instructions)));
    }

    /* Append the new instruction to the instruction list */
    prog->instructions[prog->count - 1] = new_instr;
}

static void _compile(AstNode *node, Program *prog)
{
    int i = 0;
    AstNode *x = NULL, *y = NULL, *z = NULL;
    LuciObject *addr = NULL;

    if (!node) {
        /* don't compile a NULL statement */
        puts("NOP");
        return;
    }

    switch (node->type)
    {
        case ast_stmnts_t:
            for (i = 0; i < node->data.statements.count; i++)
            {
                _compile(node->data.statements.statements[i], prog);
            }
            break;
        case ast_func_t:
            puts("PUSH params");
            _compile(node->data.funcdef.param_list, prog);
            _compile(node->data.funcdef.statements, prog);
            _compile(node->data.funcdef.ret_expr, prog);
            puts("STORE func->funcname in symtable");
            _compile(node->data.funcdef.funcname, prog);
            break;
        case ast_list_t:
            /* create a LuciObject list from the AstNode */
            add_instr(prog, CREATE, NULL, NULL, NULL);
            puts("CREATE new list");
            /* push the new list object onto the stack */
            add_instr(prog, PUSH, NULL, NULL, NULL);
            printf("PUSH new\n");
            /*
            for (i = 0; i < node->data.list.count; i++)
            {
                _compile(node->data.list.items[i], prog);
            }
            */
            break;
        case ast_while_t:
            puts("LABEL while# begin");
            puts("TEST cond? JUMP 2 : JUMP while# end");
            _compile(node->data.while_loop.cond, prog);
            _compile(node->data.while_loop.statements, prog);
            puts("JUMP to while loop#");
            puts("LABEL while# end");
            break;
        case ast_for_t:
            _compile(node->data.for_loop.list, prog);
            _compile(node->data.for_loop.statements, prog);
            _compile(node->data.for_loop.iter, prog);
            break;
        case ast_if_t:
            puts("TEST cond? JUMP 2 : JUMP else #");
            _compile(node->data.if_else.cond, prog);
            _compile(node->data.if_else.ifstatements, prog);
            puts("LABEL else#");
            _compile(node->data.if_else.elstatements, prog);
            break;
        case ast_assign_t:
            _compile(node->data.assignment.name, prog);
            _compile(node->data.assignment.right, prog);
            puts("POP -> x");
            puts("POP -> y");
            puts("DECREF y if not NULL");
            puts("ASSIGN y x");
            break;
        case ast_call_t:
            /* compile arglist, which pushes list onto stack */
            _compile(node->data.call.arglist, prog);
            /* compile funcname, which pushes symbol value onto stack */
            _compile(node->data.call.funcname, prog);
            add_instr(prog, POP, NULL, NULL, NULL);
            puts("POP -> funcname");
            puts("CALL funcname");
            break;
        case ast_listindex_t:
            _compile(node->data.listindex.list, prog);
            _compile(node->data.listindex.index, prog);
            puts("POP -> index");
            puts("POP -> list");
            puts("GET list index");
            break;
        case ast_listassign_t:
            _compile(node->data.listassign.list, prog);
            _compile(node->data.listassign.index, prog);
            _compile(node->data.listassign.right, prog);
            puts("POP -> x");
            puts("POP -> index");
            puts("POP -> list");
            puts("LASSIGN list index x");
            break;
        case ast_expr_t:
            _compile(node->data.expression.left, prog);
            _compile(node->data.expression.right, prog);
            puts("POP -> x");
            puts("POP -> y");
            puts("OP x y"); /* push result ! */
            break;
        case ast_id_t:
            printf("LOOKUP %s\n", node->data.id_val);
            printf("PUSH &%s\n", node->data.id_val);
            break;
        case ast_string_t:
            printf("CREATE new \"%s\"\n", node->data.s_val);
            printf("PUSH new\n");
            break;
        case ast_int_t:
            printf("CREATE new %ld\n", node->data.i_val);
            printf("PUSH new\n");
            break;
        case ast_float_t:
            add_instr(prog, CREATE, NULL, NULL, NULL);
            printf("CREATE new %g\n", node->data.f_val);
            printf("PUSH new\n");
            break;
        default:
            break;
    }

    return;
}

void eval(Program *)
{
    puts("Not yet implemented");
}
