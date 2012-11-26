/*
 * See Copyright Notice in luci.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "object.h"
#include "compile.h"
#include "ast.h"
#include "symbol.h"
#include "builtin.h"
#include "stack.h"

static void _compile(AstNode *, Program *);
static uint32_t push_instr(Program *, Opcode, int);
static uint32_t put_instr(Program *, uint32_t, Opcode, int);

static void add_new_loop(Program *prog, uint8_t loop_type);
static void back_patch_loop(Program *prog, uint32_t start, uint32_t end);


/**
 * Public entry point for compilation.
 *
 * Allocates a Program struct and passes it to
 * the AST walker/compiler.
 */
Program * compile_ast(AstNode *root)
{
    int i, id;
    LuciObject *o;

    if (!root) {
        die("Nothing to compile\n");
    }

    Program *prog = program_new();

    /* Add builtin symbols to symbol table */
    extern struct func_def builtins[];  /* defined in builtins.c */

    for (i = 0; builtins[i].name != 0; i++) {
        /* create object for builtin function */
        o = create_object(obj_libfunc_t);
        o->value.libfunc = builtins[i].func;
        /* add the symbol and function object to the symbol table */
        id = symtable_id(prog->locals, builtins[i].name, SYMCREATE);
        symtable_set(prog->locals, o, id);
    }

    init_variables();
    extern struct var_def globals[];
    for (i = 0; globals[i].name != 0; i++) {
        id = symtable_id(prog->locals, globals[i].name, SYMCREATE);
        symtable_set(prog->locals, globals[i].object, id);
    }

    /* compile the AST */
    _compile(root, prog);

    /* end the program with an HALT instr */
    push_instr(prog, HALT, 0);

    return prog;
}

Program *program_new(void)
{
    Program *prog = alloc(sizeof(*prog));
    prog->ip = 0;
    prog->size = BASE_INSTR_COUNT;
    prog->instructions = alloc(prog->size *
            sizeof(*prog->instructions));

    prog->locals = symtable_new(BASE_SYMTABLE_SCALE);
    prog->cotable = cotable_new(BASE_COTABLE_SIZE);

    prog->current_loop = NULL;

    return prog;
}

void program_delete(Program *prog)
{
    /* destroy each instruction, instruction list, and program */
    int i = 0;
    Instruction *instructions = prog->instructions;
    free(prog->instructions);
    symtable_delete(prog->locals);
    cotable_delete(prog->cotable);
    free(prog);
}

static uint32_t push_instr(Program *prog, Opcode op, int arg)
{
    int count = 0;
    uint32_t old_addr;

    if (!prog)
        die("Program not allocated. Can't add instruction\n");
    if (!(prog->instructions))
        die("Instruction list not allocated. Can't add instruction\n");

    /* Reallocate the program's instruction list if necessary */
    if (prog->ip + 1 > prog->size) {
        prog->size <<= 1;
        prog->instructions = realloc(prog->instructions,
                prog->size * sizeof(*(prog->instructions)));
    }

    count = put_instr(prog, prog->ip, op, arg);
    old_addr = prog->ip;
    prog->ip += count;

    return old_addr;
}

static uint32_t put_instr(Program *prog, uint32_t addr,
        Opcode op, int arg) {

    if (addr < 0 || addr > prog->ip)
        die("Address out of bounds\n");

    Instruction instr = (op << 11);
    if (op >= JUMP) {
        instr |= (0x7FF & (arg >> 16));
        prog->instructions[addr] = instr;
        instr = 0xFFFF & arg;
        prog->instructions[++addr] = instr;
        return 2;
    } else {
        instr |= (arg & 0x7FF);
        /* Append the new instruction to the instruction list */
        prog->instructions[addr] = instr;
        /* increment instruction count after appending */
        return 1;
    }
}

static void add_new_loop(Program *prog, uint8_t loop_type)
{
    struct _loop_list *loop = alloc(sizeof(*loop));
    loop->loop_type = loop_type;
    loop->breaks = NULL;
    loop->continues = NULL;
    loop->parent = prog->current_loop;

    prog->current_loop = loop;
}

static void back_patch_loop(Program *prog, uint32_t start, uint32_t end)
{
    struct _loop_jump *ptr = NULL, *old = NULL;
    struct _loop_list *cur_loop = prog->current_loop;
    struct _loop_list *parent_loop = prog->current_loop->parent;

    ptr = cur_loop->continues;
    while (ptr) {
        put_instr(prog, ptr->addr, JUMP, start);
        old = ptr;
        ptr = ptr->next;
        free(old);
    }

    /* for-loops need a POPJUMP to cleanup iterator,
     * while-loops just need a normal JUMP
     */
    Opcode break_jump = (cur_loop->loop_type == LOOP_TYPE_FOR) ? POPJUMP : JUMP;

    ptr = cur_loop->breaks;
    while (ptr) {
        put_instr(prog, ptr->addr, break_jump, end);
        old = ptr;
        ptr = ptr->next;
        free(old);
    }

    free(cur_loop);
    prog->current_loop = parent_loop;
}

static void _compile(AstNode *node, Program *prog)
{
    int i = 0;
    int a;
    int addr1, addr2;
    size_t len;
    LuciObject *obj = NULL;

    AstNode *tmp;

    if (!node) {
        /* don't compile a NULL statement */
        return;
    }

    switch (node->type)
    {
        case ast_stmnts_t:
        {
            /* compile all statements in 2 passes :/
             * 1. put all function names in symbol table
             * 2. compile all else
             */
            for (i = 0; i < node->data.statements.count; i++) {
                tmp = node->data.statements.statements[i];

                if (tmp->type == ast_func_t) {
                    /* stuff function definition names into symbol table */
                    a = symtable_id(prog->locals,
                            tmp->data.funcdef.funcname, SYMCREATE);
                }
            }

            for (i = 0; i < node->data.statements.count; i++) {
                tmp = node->data.statements.statements[i];
                /* compile the statements */
                _compile(tmp, prog);

                /* statements that leave a value on the stack (i.e.
                 * expressions lacking an assignment, or void function
                 * calls) need a post-POP statement */
                if (    (tmp->type == ast_expr_t) ||
                        (tmp->type == ast_call_t) ||
                        (tmp->type == ast_id_t)) {
                    push_instr(prog, POP, 0);
                }
            }
            break;
        }

        case ast_func_t:
        {
            printf("Compiling function\n");
            AstNode *params = node->data.funcdef.param_list;
            AstNode *id_string = NULL;

            /* Create new frame for function scope */
            Program *function = program_new();
            /* create globals table for new frame */
            function->globals = prog->locals;

            for (i = 0; i < params->data.list.count; i++) {
                id_string = params->data.list.items[i];
                /* add arg symbol to symbol table */
                a = symtable_id(function->locals, id_string->data.s, SYMCREATE);
                push_instr(function, STORE, a);
            }
            _compile(node->data.funcdef.statements, function);

            /* add a RETURN to end of function if necessary */
            if (function->instructions[function->ip - 1] != RETURN) {
                push_instr(function, PUSHNULL, 0);
                push_instr(function, RETURN, 0);
            }

            /* create function object */
            obj = create_object(obj_func_t);
            obj->value.func.frame = function;
            obj->value.func.deleter = program_delete;
            /* store function object in symbol table */
            a = symtable_id(prog->locals,
                    node->data.funcdef.funcname, SYMCREATE);
            symtable_set(prog->locals, obj, a);

            print_instructions(function);

            break;
        }

        case ast_list_t:
            /* compile list items in reverse order */
            for (i = node->data.list.count - 1; i >= 0; i--) {
                /* compile each list member */
                _compile(node->data.list.items[i], prog);
            }
            /* add MKLIST instruction for # of list items */
            push_instr(prog, MKLIST, node->data.list.count);
            break;

        case ast_while_t:
            add_new_loop(prog, LOOP_TYPE_WHILE);
            /* store addr of start of while */
            addr1 = prog->ip;
            /* compile test expression */
            _compile(node->data.while_loop.cond, prog);
            /* push a bogus jump instr */
            addr2 = push_instr(prog, JUMP, -1);
            /* compile body of while loop */
            _compile(node->data.while_loop.statements, prog);
            /* add a jump to beginning of while loop */
            push_instr(prog, JUMP, addr1);
            /* change bogus instr to conditional jump */
            put_instr(prog, addr2, JUMPZ, prog->ip);

            back_patch_loop(prog, addr1, prog->ip);
            break;

        case ast_for_t:
            add_new_loop(prog, LOOP_TYPE_FOR);
            /* compile loop (expr) */
            _compile(node->data.for_loop.list, prog);
            /* Make Iterator */
            push_instr(prog, MKITER, 0);
            /* store addr of start of for-loop */
            addr1 = prog->ip;
            /* push bogus jump for getting iterator->next */
            addr2 = push_instr(prog, JUMP, -1);
            /* store iterator output in symbol */
            a = symtable_id(prog->locals, node->data.for_loop.iter, SYMCREATE);
            push_instr(prog, STORE, a);
            /* compile body of for-loop */
            _compile(node->data.for_loop.statements, prog);
            /* add jump to beginning of for-loop */
            push_instr(prog, JUMP, addr1);
            /* change bogus jump to a JUMPI (get iter->next or jump) */
            put_instr(prog, addr2, ITERJUMP, prog->ip);

            back_patch_loop(prog, addr1, prog->ip);
            break;

        case ast_if_t:
            /* compile test expression */
            _compile(node->data.if_else.cond, prog);
            /* add bogus instruction number 1 */
            addr1 = push_instr(prog, JUMP, -1);
            /* compile TRUE statements */
            _compile(node->data.if_else.ifstatements, prog);

            if (node->data.if_else.elstatements) {
                /* add bogus instruction number 2 */
                addr2 = push_instr(prog, JUMP, -1);
                /* change bogus instr 1 to a conditional jump */
                put_instr(prog, addr1, JUMPZ, prog->ip);
                /* compile FALSE statements */
                _compile(node->data.if_else.elstatements, prog);
                /* change bogus instr 2 to a jump */
                put_instr(prog, addr2, JUMP, prog->ip);
            }
            else {
                /* change bogus instr 1 to a conditional jump */
                put_instr(prog, addr1, JUMPZ, prog->ip);
            }
            break;

        case ast_assign_t:
            tmp = node->data.assignment.right;
            /* traverse nested assignments until we reach RH value */
            while (tmp->type == ast_assign_t) {
                tmp = tmp->data.assignment.right;
            }
            /* compile actual value of assignment */
            _compile(tmp, prog);
            tmp = node;
            /* duplicate top-of-stack for each additional assignment */
            while (tmp->data.assignment.right->type == ast_assign_t) {
                push_instr(prog, DUP, 0);
                /* get id of left-hand symbol */
                a = symtable_id(prog->locals, tmp->data.assignment.name, SYMCREATE);
                /* add a STORE instruction */
                push_instr(prog, STORE, a);

                tmp = tmp->data.assignment.right;
            }
            /* get id of final left-hand symbol */
            a = symtable_id(prog->locals, tmp->data.assignment.name, SYMCREATE);
            /* add final STORE instruction */
            push_instr(prog, STORE, a);
            break;


        case ast_call_t:
            /* compile arglist, which pushes each arg onto stack */
            tmp = node->data.call.arglist;
            for (i = 0; i < tmp->data.list.count; i++) {
                _compile(tmp->data.list.items[i], prog);
            }
            /* compile funcname, which pushes symbol value onto stack */
            _compile(node->data.call.funcname, prog);
            /* add CALL instr, specifying # of args */
            push_instr(prog, CALL, tmp->data.list.count);
            break;

        case ast_listaccess_t:
            _compile(node->data.listaccess.index, prog);
            _compile(node->data.listaccess.list, prog);
            push_instr(prog, LISTGET, 0);
            break;

        case ast_listassign_t:
            _compile(node->data.listassign.right, prog);
            _compile(node->data.listassign.index, prog);
            _compile(node->data.listassign.list, prog);
            push_instr(prog, LISTPUT, 0);
            break;

        case ast_expr_t:
            _compile(node->data.expression.left, prog);
            _compile(node->data.expression.right, prog);
            a = node->data.expression.op;
            /* make long immediate from int op */
            push_instr(prog, BINOP, a);
            /* yak("BINOP %d\n", a); */
            break;

        case ast_id_t:
            a = symtable_id(prog->locals, node->data.id.val, SYMFIND);
            if (a < 0) {
                a = symtable_id(prog->globals, node->data.id.val, SYMFIND);
                if (a < 0) {
                    die("%s undefined.\n", node->data.id.val);
                }
                /* else */
                push_instr(prog, LOADG, a);
            } else {
                push_instr(prog, LOADS, a);
            }

            break;

        case ast_string_t:
            obj = create_object(obj_str_t);
            obj->value.s = strndup(node->data.s, strlen(node->data.s));
            a = constant_id(prog->cotable, obj);
            push_instr(prog, LOADK, a);
            /* yak("LOADK \"%s\"\n", constant.val.s); */
            break;

        case ast_float_t:
            obj = create_object(obj_float_t);
            obj->value.f = node->data.f;
            a = constant_id(prog->cotable, obj);
            push_instr(prog, LOADK, a);
            /* yak("LOADK %g\n", constant.val.f); */
            break;

        case ast_integer_t:
            obj = create_object(obj_int_t);
            obj->value.i = node->data.i;
            a = constant_id(prog->cotable, obj);
            push_instr(prog, LOADK, a);
            /* yak("LOADK %ld\n", constant.val.i); */
            break;

        case ast_return_t:
            if (node->data.return_stmt.expr == NULL) {
                push_instr(prog, PUSHNULL, 0);
            } else {
                _compile(node->data.return_stmt.expr, prog);
            }
            push_instr(prog, RETURN, 0);
            break;

        case ast_break_t:
        {
            struct _loop_jump *jmp = alloc(sizeof(*jmp));
            struct _loop_jump *ptr;

            if (!prog->current_loop) {
                die("'break' @ line %d not inside a loop\n", node->lineno);
            }
            ptr = prog->current_loop->breaks;
            /* push a bogus JUMP instr to be backpatched later */
            jmp->addr = push_instr(prog, JUMP, -1);
            jmp->next = NULL;
            if (!ptr) {
                /* make new break addr head of break list */
                prog->current_loop->breaks = jmp;
            } else {
                /* walk break list to the end */
                while (ptr->next) {
                    ptr = ptr->next;
                }
                ptr->next = jmp;
            }
            break;
        }

        case ast_continue_t:
        {
            struct _loop_jump *jmp = alloc(sizeof(*jmp));
            struct _loop_jump *ptr;
            if (!prog->current_loop) {
                die("'continue' @ line %d not inside a loop\n", node->lineno);
            }
            ptr = prog->current_loop->continues;
            /* push a bogus JUMP instr to be backpatched later */
            jmp->addr = push_instr(prog, JUMP, -1);
            if (!ptr) {
                /* make new continue addr head of continue list */
                prog->current_loop->continues = jmp;
            } else {
                /* walk continue list to the end */
                while (ptr->next) {
                    ptr = ptr->next;
                }
                ptr->next = jmp;
            }
            break;
        }

        case ast_pass_t:
            push_instr(prog, NOP, 0);
            break;
    }
    return;
}

static char *instruction_names[] = {
    "NOP",
    "POP",
    "PUSHNULL",
    "LOADK",
    "LOADS",
    "LOADG",
    "DUP",
    "STORE",
    "BINOP",
    "CALL",
    "RETURN",
    "MKLIST",
    "LISTGET",
    "LISTPUT",
    "MKITER",
    "HALT",
    /* here begins extended length instructions */
    "JUMP",
    "POPJUMP",
    "JUMPZ",
    "ITERJUMP",
};
/*
 * Prints string representations of each instruction in the program.
 * Used for debugging (or fun)
 */
void print_instructions(Program *prog)
{
    int i, a, instr;
    const char *name = NULL;

    for (i = 0; i < prog->ip; i ++) {
        a = prog->instructions[i] & 0x7FF;
        instr = prog->instructions[i] >> 11;

        /* rip out another instr if extended */
        if (instr >= JUMP) {
            a = prog->instructions[i + 1] + (a << 16);
        }
        printf("%03x: %s 0x%x\n", i, instruction_names[instr], a);
        /* increment 'i' if we just printed an extended instruction */
        i += (instr >= JUMP) ? 1 : 0;
    }
}
