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
static int add_instr(Program *, Opcode, int);
static void change_instr(Program *, int, Opcode, int);

static char *instr_names[] = {
    "NOP",
    "LOADK",
    "LOADS",
    "STORE",
    "BINOP",
    "CALL",
    "JUMP",
    "JUMPZ",
    "MKLIST",
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

    Program *prog = program_new();

    /* Add builtin symbols to symbol table */
    int i, id;
    LuciObject *o;
    extern struct func_def builtins[];

    for (i = 0; builtins[i].name != 0; i++)
    {
        /* create object for builtin function */
        o = create_object(obj_func_t);
        o->value.func = builtins[i].func;
        /* add the symbol and function object to the symbol table */
        id = symbol_id(prog->symtable, builtins[i].name);
        symtable_set(prog->symtable, o, id);
    }

    /* compile the AST */
    _compile(root, prog);

    /* end the program with an EXIT instr */
    add_instr(prog, EXIT, 0);

    return prog;
}

Program *program_new(void)
{
    Program *prog = alloc(sizeof(*prog));
    prog->count = 0;
    prog->size = BASE_INSTR_COUNT;
    prog->instructions = alloc(prog->size *
            sizeof(*prog->instructions));

    prog->symtable = symtable_new(BASE_SYMTABLE_SIZE);
    prog->cotable = cotable_new(BASE_COTABLE_SIZE);

    return prog;
}

void program_delete(Program *prog)
{
    /* destroy each instruction, instruction list, and program */
    int i = 0;
    Instruction *instructions = prog->instructions;
    free(prog->instructions);
    symtable_delete(prog->symtable);
    cotable_delete(prog->cotable);
    free(prog);
}

static int add_instr(Program *prog, Opcode op, int arg)
{
    if (!prog)
        die("Program not allocated. Can't add instruction\n");
    if (!(prog->instructions))
        die("Instruction list not allocated. Can't add instruction\n");

    /* Allocate and initialize the instruction */
    Instruction instr = (op << 11) | (arg & 0x7FF);

    /* Reallocate the program's instruction list if necessary */
    if (prog->count > prog->size) {
        prog->size <<= 1;
        prog->instructions = realloc(prog->instructions,
                prog->size * sizeof(*(prog->instructions)));
    }

    /* Append the new instruction to the instruction list */
    prog->instructions[prog->count] = instr;

    /* increment instruction count after appending */
    return prog->count ++;
}

static void change_instr(Program *prog, int addr, Opcode op, int arg)
{
    if (addr < 0 || addr > prog->count)
        die("Address out of bounds\n");

    Instruction instr = (op << 11) | (arg & 0x7FF);

    prog->instructions[addr] = instr;
}

static void _compile(AstNode *node, Program *prog)
{
    int i = 0;
    int a;
    int addr1, addr2;
    size_t len;
    LuciObject *obj = NULL;

    AstConstant constant;

    if (!node) {
        /* don't compile a NULL statement */
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
            for (i = 0; i < node->data.list.count; i++)
            {
                /* compile each list member */
                _compile(node->data.list.items[i], prog);
            }
            /* add MKLIST instruction for # of list items */
            add_instr(prog, MKLIST, node->data.list.count);
            break;
        case ast_while_t:
            /* store addr of start of while */
            addr1 = prog->count;
            /* compile test expression */
            _compile(node->data.while_loop.cond, prog);
            /* push a bogus instr */
            addr2 = add_instr(prog, NOP, 0);
            /* compile body of while loop */
            _compile(node->data.while_loop.statements, prog);
            /* add a jump to beginning of while loop */
            add_instr(prog, JUMP, addr1);
            /* change bogus instr to conditional jump */
            change_instr(prog, addr2, JUMPZ, prog->count);
            break;
        case ast_for_t:
            _compile(node->data.for_loop.list, prog);
            _compile(node->data.for_loop.statements, prog);
            _compile(node->data.for_loop.iter, prog);
            break;
        case ast_if_t:
            /* compile test expression */
            _compile(node->data.if_else.cond, prog);
            /* add bogus instruction number 1 */
            addr1 = add_instr(prog, NOP, 0);
            /* compile TRUE statements */
            _compile(node->data.if_else.ifstatements, prog);

            if (node->data.if_else.elstatements) {
                /* add bogus instruction number 2 */
                addr2 = add_instr(prog, NOP, 0);
                /* change bogus instr 1 to a conditional jump */
                change_instr(prog, addr1, JUMPZ, prog->count);
                /* compile FALSE statements */
                _compile(node->data.if_else.elstatements, prog);
                /* change bogus instr 2 to a jump */
                change_instr(prog, addr2, JUMP, prog->count);
            }
            else {
                /* change bogus instr 1 to a conditional jump */
                change_instr(prog, addr1, JUMPZ, prog->count);
            }
            break;
        case ast_assign_t:
            /* compile the right-hand value */
            _compile(node->data.assignment.right, prog);
            /* get id of left-hand symbol */
            a = symbol_id(prog->symtable, node->data.assignment.name);
            /* add a STORE instruction */
            add_instr(prog, STORE, a);
            break;
        case ast_call_t:
            /* compile arglist, which pushes list onto stack */
            _compile(node->data.call.arglist, prog);
            /* compile funcname, which pushes symbol value onto stack */
            _compile(node->data.call.funcname, prog);
            add_instr(prog, CALL, 0);
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
            add_instr(prog, BINOP, a);
            yak("BINOP %d\n", a);
            break;
        case ast_id_t:
            a = symbol_id(prog->symtable, node->data.id.val);
            add_instr(prog, LOADS, a);
            yak("LOADS %s\n", node->data.id.val);
            break;
        case ast_constant_t:
            constant = node->data.constant;
            switch (constant.type) {
                case co_string_t:
                    obj = create_object(obj_str_t);
                    obj->value.s_val = strndup(constant.val.s,
                            strlen(constant.val.s));

                    a = constant_id(prog->cotable, obj);
                    add_instr(prog, LOADK, a);
                    yak("LOADK \"%s\"\n", constant.val.s);
                    break;
                case co_float_t:
                    obj = create_object(obj_float_t);
                    obj->value.f_val = constant.val.f;
                    a = constant_id(prog->cotable, obj);
                    add_instr(prog, LOADK, a);
                    yak("LOADK %g\n", constant.val.f);
                    break;
                case co_int_t:
                    obj = create_object(obj_int_t);
                    obj->value.i_val = constant.val.i;
                    a = constant_id(prog->cotable, obj);
                    add_instr(prog, LOADK, a);
                    yak("LOADK %ld\n", constant.val.i);
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
        printf("%03x: %s %x\n", i,
                instr_names[prog->instructions[i] >> 11],
                prog->instructions[i] & 0x7FF);
}
