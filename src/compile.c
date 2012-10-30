#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "object.h"
#include "compile.h"
#include "ast.h"
#include "symbol.h"
#include "functions.h"
#include "stack.h"

static void _compile(AstNode *, Program *);
static void add_instr(Program *, Opcode, int, int);

static char *instr_names[] = {
    "NOP",
    "LOADK",
    "LOADS",
    "STORE",
    "BINOP",
    "CALL",
    "JUMPL",
    "JUMPN",
    "EXIT"
};

/**
 * Public entry point for compilation.
 *
 * Allocates a Program struct and passes it to
 * the AST walker/compiler.
 */
Program * compile_ast(AstNode *root)
{
    if (!root)
        die("Nothing to compile\n");

    Program *prog = alloc(sizeof(*prog));
    prog->count = 0;
    prog->size = BASE_INSTR_COUNT;
    prog->instructions = alloc(prog->size * sizeof(*(prog->instructions)));

    prog->symtable = symtable_new(0xFFF);

    prog->cotable = cotable_new(0xFF);

    /* compile the AST */
    _compile(root, prog);

    /* end the program with an EXIT instr */
    add_instr(prog, EXIT, 0, 0);

    return prog;
}

void destroy_program(Program *prog)
{
    /* destroy each instruction, instruction list, and program */
    int i = 0;
    Instruction **instructions = prog->instructions;
    for (i = 0; i < prog->count; i ++) {
        free(instructions[i]);
    }
    free(prog->instructions);
    symtable_delete(prog->symtable);
    cotable_delete(prog->cotable);
    free(prog);
}

static void add_instr(Program *prog, Opcode op, int a, int b)
{
    if (!prog)
        die("Program not allocated. Can't add instruction\n");
    if (!(prog->instructions))
        die("Instruction list not allocated. Can't add instruction\n");

    /* Allocate and initialize the instruction */
    Instruction *new_instr = alloc(sizeof(*new_instr));
    new_instr->opcode = op;
    new_instr->a = a;
    new_instr->b = b;

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
    int a, b;
    size_t len;
    LuciObject *obj = NULL;

    if (!node) {
        /* don't compile a NULL statement */
        yak("NOP");
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
            _compile(node->data.funcdef.param_list, prog);
            _compile(node->data.funcdef.statements, prog);
            _compile(node->data.funcdef.funcname, prog);
            break;
        case ast_list_t:
            /* create a LuciObject list from the AstNode */
            /* push the new list object onto the stack */
            for (i = 0; i < node->data.list.count; i++)
            {
                _compile(node->data.list.items[i], prog);
            }
            break;
        case ast_while_t:
            yak("LABEL while# begin");
            yak("TEST cond? JUMP 2 : JUMP while# end");
            _compile(node->data.while_loop.cond, prog);
            _compile(node->data.while_loop.statements, prog);
            yak("JUMP to while loop#");
            yak("LABEL while# end");
            break;
        case ast_for_t:
            _compile(node->data.for_loop.list, prog);
            _compile(node->data.for_loop.statements, prog);
            _compile(node->data.for_loop.iter, prog);
            break;
        case ast_if_t:
            yak("TEST cond? JUMP 2 : JUMP else #");
            _compile(node->data.if_else.cond, prog);
            _compile(node->data.if_else.ifstatements, prog);
            yak("LABEL else#");
            _compile(node->data.if_else.elstatements, prog);
            break;
        case ast_assign_t:
            _compile(node->data.assignment.right, prog);
            a = symbol_id(prog->symtable, node->data.assignment.name);
            add_instr(prog, STORE, a, 0);
            yak("STORE %d\n", a);
            break;
        case ast_call_t:
            /* compile arglist, which pushes list onto stack */
            _compile(node->data.call.arglist, prog);
            /* compile funcname, which pushes symbol value onto stack */
            _compile(node->data.call.funcname, prog);
            add_instr(prog, CALL, 0, 0);
            yak("CALL");
            break;
        case ast_listindex_t:
            _compile(node->data.listindex.list, prog);
            _compile(node->data.listindex.index, prog);
            break;
        case ast_listassign_t:
            /* _compile(node->data.listassign.name, prog); */
            _compile(node->data.listassign.index, prog);
            _compile(node->data.listassign.right, prog);
            break;
        case ast_expr_t:
            _compile(node->data.expression.left, prog);
            _compile(node->data.expression.right, prog);
            a = node->data.expression.op;
            /* make long immediate from int op */
            add_instr(prog, BINOP, a, 0);
            yak("BINOP %d\n", a);
            break;
        case ast_id_t:
            a = symbol_id(prog->symtable, node->data.id.val);
            add_instr(prog, LOADS, a, 0);
            yak("LOADS %s\n", node->data.id.val);
            break;
        case ast_constant_t:
            switch (node->data.constant.type) {
                case co_string_t:
                    obj = create_object(obj_str_t);
                    len = strlen(node->data.constant.val.s);
                    obj->value.s_val = alloc(len + 1);
                    strncpy(obj->value.s_val, node->data.constant.val.s, len);
                    obj->value.s_val[len] = '\0';
                    a = constant_id(prog->cotable, obj);
                    add_instr(prog, LOADK, a, 0);
                    yak("LOADK \"%s\"\n", node->data.constant.val.s);
                    break;
                case co_float_t:
                    obj = create_object(obj_float_t);
                    obj->value.f_val = node->data.constant.val.f;
                    a = constant_id(prog->cotable, obj);
                    add_instr(prog, LOADK, a, 0);
                    yak("LOADK %g\n", node->data.constant.val.f);
                    break;
                case co_int_t:
                    obj = create_object(obj_int_t);
                    obj->value.i_val = node->data.constant.val.i;
                    a = constant_id(prog->cotable, obj);
                    add_instr(prog, LOADK, a, 0);
                    yak("LOADK %ld\n", node->data.constant.val.i);
                    break;
                default:
                    die("Bad constant type\n");
            }
            break;
    }

    return;
}

void print_instructions(Program *prog)
{
    int i = 0;
    for (i = 0; i < prog->count; i ++)
        printf("%03x: %s %d %d\n", i,
                instr_names[prog->instructions[i]->opcode],
                prog->instructions[i]->a, prog->instructions[i]->b);
}
