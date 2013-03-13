/*
 * See Copyright Notice in luci.h
 */

/**
 * @file compile.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "luci.h"
#include "object.h"
#include "compile.h"
#include "ast.h"
#include "symbol.h"
#include "builtin.h"
#include "stack.h"

static void compile(AstNode *, CompileState *);

static uint32_t push_instr(CompileState *, Opcode, int);
static uint32_t put_instr(CompileState *, uint32_t, Opcode, int);

static void add_new_loop(CompileState *cs, int loop_type);
static void back_patch_loop(CompileState *cs, uint32_t start, uint32_t end);


/**
 * Compile an integer constant AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_int_constant(AstNode *node, CompileState *cs)
{
    int a;
    LuciObject *obj;

    obj = LuciInt_new(node->data.i);
    a = constant_id(cs->ctable, obj);
    push_instr(cs, LOADK, a);
}

/**
 * Compile a floating-point constant AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_float_constant(AstNode *node, CompileState *cs)
{
    int a;
    LuciObject *obj;

    obj = LuciFloat_new(node->data.f);
    a = constant_id(cs->ctable, obj);
    push_instr(cs, LOADK, a);
}

/**
 * Compile a string constant AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_string_constant(AstNode *node, CompileState *cs)
{
    int a;
    LuciObject *obj;

    /* TODO: Free original string from AST Node since it's
     * copied on object creation?
     * */
    obj = LuciString_new(strdup(node->data.s));
    a = constant_id(cs->ctable, obj);
    push_instr(cs, LOADK, a);
}

/**
 * Compile a symbol/id AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_id_expr(AstNode *node, CompileState *cs)
{
    int a;
    a = symtable_id(cs->ltable, node->data.id.val, SYMFIND);
    if (a >= 0) {
        /* found the symbol immediately */
        push_instr(cs, LOADS, a);
    }
    else {
        /* didn't find symbol in the locals symbol table */
        /* if there isn't a globals table, the symbol doesn't exist */
        if (cs->gtable == NULL) {
            DIE("%s undefined.\n", node->data.id.val);
        } else {
            /* search the globals table for the symbol */
            a = symtable_id(cs->gtable, node->data.id.val, SYMFIND);
            if (a < 0) {
                /* not in globals table either */
                DIE("%s undefined.\n", node->data.id.val);
            } else {
                push_instr(cs, LOADG, a);
            }
        }
    }
}


/**
 * Compile a binary expression AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_binary_expr(AstNode *node, CompileState *cs)
{
    int a;
    compile(node->data.expression.left, cs);
    compile(node->data.expression.right, cs);
    a = node->data.expression.op;
    push_instr(cs, BINOP, a);
}

/**
 * Compile a container access AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_container_access(AstNode *node, CompileState *cs)
{
    compile(node->data.contaccess.index, cs);
    compile(node->data.contaccess.container, cs);
    push_instr(cs, CGET, 0);
}

/**
 * Compile a container assignment AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_container_assignment(AstNode *node, CompileState *cs)
{
    compile(node->data.contassign.right, cs);
    compile(node->data.contassign.index, cs);
    compile(node->data.contassign.container, cs);
    push_instr(cs, CPUT, 0);
}

/**
 * Compile a map definition AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_map_def(AstNode *node, CompileState *cs)
{
    int i;
    /* Compile map key-value pairs in reverse order */
    for (i = node->data.mapdef.count - 1; i >= 0; i--) {
        /* compile each pair */
        compile(node->data.mapdef.pairs[i], cs);
    }
    /* add MKMAP instruction for # of k-v pairs */
    push_instr(cs, MKMAP, node->data.mapdef.count);
}

/**
 * Compile a map key-value pair AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_map_keyval(AstNode *node, CompileState *cs)
{
    compile(node->data.mapkeyval.key, cs);
    compile(node->data.mapkeyval.val, cs);
}

/**
 * Compile a list definition AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_list_def(AstNode *node, CompileState *cs)
{
    int i;
    /* compile list items in reverse order */
    for (i = node->data.listdef.count - 1; i >= 0; i--) {
        /* compile each list member */
        compile(node->data.listdef.items[i], cs);
    }
    /* add MKLIST instruction for # of list items */
    push_instr(cs, MKLIST, node->data.listdef.count);
}

/**
 * Compile an assignment AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_assignment(AstNode *node, CompileState *cs)
{
    int a;
    AstNode *tmp;

    tmp = node->data.assignment.right;
    /* traverse nested assignments until we reach RH value */
    while (tmp->type == ast_assign_t) {
        tmp = tmp->data.assignment.right;
    }
    /* compile actual value of assignment */
    compile(tmp, cs);
    tmp = node;
    /* duplicate top-of-stack for each additional assignment */
    while (tmp->data.assignment.right->type == ast_assign_t) {
        push_instr(cs, DUP, 0);
        /* get id of left-hand symbol */
        a = symtable_id(cs->ltable, tmp->data.assignment.name, SYMCREATE);
        /* add a STORE instruction */
        push_instr(cs, STORE, a);

        tmp = tmp->data.assignment.right;
    }
    /* get id of final left-hand symbol */
    a = symtable_id(cs->ltable, tmp->data.assignment.name, SYMCREATE);
    /* add final STORE instruction */
    push_instr(cs, STORE, a);
}

/**
 * Compile a while-loop AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_while_loop(AstNode *node, CompileState *cs)
{
    uint32_t addr1, addr2;

    add_new_loop(cs, LOOP_TYPE_WHILE);
    /* store addr of start of while */
    addr1 = cs->instr_count;
    /* compile test expression */
    compile(node->data.while_loop.cond, cs);
    /* push a bogus jump instr */
    addr2 = push_instr(cs, JUMP, -1);
    /* compile body of while loop */
    compile(node->data.while_loop.statements, cs);
    /* add a jump to beginning of while loop */
    push_instr(cs, JUMP, addr1);
    /* change bogus instr to conditional jump */
    put_instr(cs, addr2, JUMPZ, cs->instr_count);

    back_patch_loop(cs, addr1, cs->instr_count);
}

/**
 * Compile a for-loop AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_for_loop(AstNode *node, CompileState *cs)
{
    uint32_t addr1, addr2;
    int a;

    add_new_loop(cs, LOOP_TYPE_FOR);
    /* compile loop (expr) */
    compile(node->data.for_loop.container, cs);
    /* Make Iterator */
    push_instr(cs, MKITER, 0);
    /* store addr of start of for-loop */
    addr1 = cs->instr_count;
    /* push bogus jump for getting iterator->next */
    addr2 = push_instr(cs, JUMP, -1);
    /* store iterator output in symbol */
    a = symtable_id(cs->ltable, node->data.for_loop.iter, SYMCREATE);
    push_instr(cs, STORE, a);
    /* compile body of for-loop */
    compile(node->data.for_loop.statements, cs);
    /* add jump to beginning of for-loop */
    push_instr(cs, JUMP, addr1);
    /* change bogus jump to a JUMPI (get iter->next or jump) */
    put_instr(cs, addr2, ITERJUMP, cs->instr_count);

    back_patch_loop(cs, addr1, cs->instr_count);
}

/**
 * Compile an if-else block AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_if_else(AstNode *node, CompileState *cs)
{
    uint32_t addr1, addr2;

    /* compile test expression */
    compile(node->data.if_else.cond, cs);
    /* add bogus instruction number 1 */
    addr1 = push_instr(cs, JUMP, -1);
    /* compile TRUE statements */
    compile(node->data.if_else.ifstatements, cs);

    if (node->data.if_else.elstatements) {
        /* add bogus instruction number 2 */
        addr2 = push_instr(cs, JUMP, -1);
        /* change bogus instr 1 to a conditional jump */
        put_instr(cs, addr1, JUMPZ, cs->instr_count);
        /* compile FALSE statements */
        compile(node->data.if_else.elstatements, cs);
        /* change bogus instr 2 to a jump */
        put_instr(cs, addr2, JUMP, cs->instr_count);
    }
    else {
        /* change bogus instr 1 to a conditional jump */
        put_instr(cs, addr1, JUMPZ, cs->instr_count);
    }

}

/**
 * Compile a function call AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_func_call(AstNode *node, CompileState *cs)
{
    int i;
    /* compile arglist, which pushes each arg onto stack */
    AstNode *tmp = node->data.call.arglist;
    for (i = 0; i < tmp->data.listdef.count; i++) {
        compile(tmp->data.listdef.items[i], cs);
    }
    /* compile funcname, which pushes symbol value onto stack */
    compile(node->data.call.funcname, cs);
    /* add CALL instr, specifying # of args */
    push_instr(cs, CALL, tmp->data.listdef.count);
}

/**
 * Compile a function definition AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_func_def(AstNode *node, CompileState *cs)
{
    int i, a, nparams;
    AstNode *params = node->data.funcdef.param_list;
    AstNode *id_string = NULL;
    LuciObject *obj = NULL;

    /* Create new frame for function scope */
    CompileState *func_cs = CompileState_new();

    /* create globals table for new frame */
    /* !NOTE: in order to support nested function definitions,
     * this globals table needs to include BOTH the parent function's
     * locals table and globals table */
    func_cs->gtable = cs->ltable;

    nparams = params->data.listdef.count;
    /* add each parameter to symbol table */
    for (i = 0; i < nparams; i++) {
        id_string = params->data.listdef.items[i];
        /* add arg symbol to symbol table */
        a = symtable_id(func_cs->ltable, id_string->data.s, SYMCREATE);
    }
    compile(node->data.funcdef.statements, func_cs);

    /* add a RETURN to end of function if necessary */
    if (func_cs->instructions[func_cs->instr_count - 1] != RETURN) {
        push_instr(func_cs, PUSHNULL, 0);
        push_instr(func_cs, RETURN, 0);
    }

    /* create function object */
    obj = LuciFunction_new(Frame_from_CompileState(func_cs, nparams));

    if (obj == NULL) {
        puts("NULL\n");
    }

    /*
    printf("#### %s %d ####\n", node->data.funcdef.funcname, a);
    print_instructions(func_cs);
    puts("############");
    */

    /* Clean up CompileState created to compile this function */
    CompileState_delete(func_cs);

    /* store function object in symbol table */
    a = symtable_id(cs->ltable, node->data.funcdef.funcname, SYMCREATE);
    symtable_set(cs->ltable, obj, a);
}

/**
 * Compile a statements block AST Node
 *
 * compile all statements in 3 passes :'(
 * 1. put all function names in symbol table
 * 2. compile all global statements
 * 3. compile all statements inside of function defs
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_statements(AstNode *node, CompileState *cs)
{
    int i, t;
    AstNode *tmp;

    t = node->data.statements.count;
    /* 1. Put all function declarations in global symbol table */
    for (i = 0; i < t; i++) {
        tmp = node->data.statements.statements[i];

        if (tmp->type == ast_func_t) {
            /* stuff function definition names into symbol table */
            int a = symtable_id(cs->ltable, tmp->data.funcdef.funcname, SYMCREATE);
            LUCI_DEBUG("Stuff symbol %s (%d)\n", tmp->data.funcdef.funcname, a);
        }
    }

    /* 2. Compile global statements */
    for (i = 0; i < t; i++) {
        tmp = node->data.statements.statements[i];
        /* compile global statements */
        if (tmp->type != ast_func_t) {
            compile(tmp, cs);

            /* statements that leave a value on the stack (i.e.
             * expressions lacking an assignment, or void function
             * calls) need a post-POP statement */
            if ((tmp->type == ast_expr_t) || (tmp->type == ast_call_t) ||
                    (tmp->type == ast_id_t)) {
                push_instr(cs, POP, 0);
            }
        }
    }

    /* 3. Compile function definitions */
    for (i = 0; i < t; i++) {
        tmp = node->data.statements.statements[i];
        if (tmp->type == ast_func_t) {
            compile(tmp, cs);
        }
    }
}

/**
 * Compile a @code break @endcode AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_break(AstNode *node, CompileState *cs)
{
    struct loop_jump *jmp = alloc(sizeof(*jmp));
    struct loop_jump *ptr;

    if (!cs->current_loop) {
        DIE("'break' @ line %d not inside a loop\n", node->lineno);
    }
    ptr = cs->current_loop->breaks;
    /* push a bogus JUMP instr to be backpatched later */
    jmp->addr = push_instr(cs, JUMP, -1);
    jmp->next = NULL;
    if (!ptr) {
        /* make new break addr head of break list */
        cs->current_loop->breaks = jmp;
    } else {
        /* walk break list to the end */
        while (ptr->next) {
            ptr = ptr->next;
        }
        ptr->next = jmp;
    }
}

/**
 * Compile a @code continue @endcode AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_continue(AstNode *node, CompileState *cs)
{
    struct loop_jump *jmp = alloc(sizeof(*jmp));
    struct loop_jump *ptr;
    if (!cs->current_loop) {
        DIE("'continue' @ line %d not inside a loop\n", node->lineno);
    }
    ptr = cs->current_loop->continues;
    /* push a bogus JUMP instr to be backpatched later */
    jmp->addr = push_instr(cs, JUMP, -1);
    if (!ptr) {
        /* make new continue addr head of continue list */
        cs->current_loop->continues = jmp;
    } else {
        /* walk continue list to the end */
        while (ptr->next) {
            ptr = ptr->next;
        }
        ptr->next = jmp;
    }
}

/**
 * Compile a @code return @endcode AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_return(AstNode *node, CompileState *cs)
{
    if (node->data.return_stmt.expr == NULL) {
        push_instr(cs, PUSHNULL, 0);
    } else {
        compile(node->data.return_stmt.expr, cs);
    }
    push_instr(cs, RETURN, 0);
}

/**
 * Compile a @code pass @endcode AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_pass(AstNode *node, CompileState *cs)
{
    push_instr(cs, NOP, 0);
}

/**
 * Array of compiler functions for each type of AST Node
 */
static void (*compilers[])(AstNode *, CompileState *) = {
    compile_int_constant,
    compile_float_constant,
    compile_string_constant,
    compile_id_expr,
    compile_binary_expr,
    compile_container_access,
    compile_container_assignment,
    compile_map_def,
    compile_map_keyval,
    compile_list_def,
    compile_assignment,
    compile_while_loop,
    compile_for_loop,
    compile_if_else,
    compile_func_call,
    compile_func_def,
    compile_statements,
    compile_break,
    compile_continue,
    compile_return,
    compile_pass
};


/**
 * Dispatches compilation for each type of AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile(AstNode *node, CompileState *cs)
{
    compilers[node->type](node, cs);
}

/**
 * Public entry point for compilation.
 *
 * Allocates a CompileState struct and passes it to
 * the AST walker/compiler.
 *
 * @param cs existing CompileState for use in Luci's interactive mode
 * @param root top-level AST Node
 * @returns complete CompileState
 */
CompileState * compile_ast(CompileState *cs, AstNode *root)
{
    int i, id;
    LuciObject *obj;

    if (!root) {
        DIE("%s", "Nothing to compile\n");
    }

    /* if we're compiling an AST from scratch, create a new
     * CompileState to pass around
     */
    if (cs == NULL) {
        cs = CompileState_new();
        /* Add builtin symbols to symbol table */
        extern struct func_def builtins[];  /* defined in builtins.c */

        for (i = 0; builtins[i].name != 0; i++) {
            /* create object for builtin function */
            obj = LuciLibFunc_new(builtins[i].func);
            /* add the symbol and function object to the symbol table */
            id = symtable_id(cs->ltable, builtins[i].name, SYMCREATE);
            symtable_set(cs->ltable, obj, id);
        }

        init_variables();
        extern struct var_def globals[];
        for (i = 0; globals[i].name != 0; i++) {
            id = symtable_id(cs->ltable, globals[i].name, SYMCREATE);
            symtable_set(cs->ltable, globals[i].object, id);
        }
    }
    /* otherwise, cleanup the old instructions but maintain the symbol table
     * and so on
     */
    else {
        cs = CompileState_refresh(cs);
    }

    /* compile the AST */
    compile(root, cs);

    /* end the CompileState with an HALT instr */
    push_instr(cs, HALT, 0);

    return cs;
}

/**
 * Return a copy of the given Frame
 *
 * @param f Frame to copy
 * @returns copy of the given Frame
 */
Frame *Frame_copy(Frame *f)
{
    int i;
    Frame *copy = NULL;
    LuciObject **locals = NULL;

    if (f == NULL) {
        DIE("%s", "Can't copy NULL frame\n");
    }

    copy = alloc(sizeof(*copy));

    copy->nparams = f->nparams;
    copy->nlocals = f->nlocals;
    copy->nconstants = f->nconstants;
    copy->ip = f->ip;
    copy->ninstrs = f->ninstrs;
    copy->instructions = f->instructions;
    copy->globals = f->globals;
    copy->constants = f->constants;

    LUCI_DEBUG("Copying frame:\nnparams: %d\nnlocals: %d\nnconstants: %d\n",
            copy->nparams, copy->nlocals, copy->nconstants);

    /* Copy the frame's local variable array */
    locals = alloc(copy->nlocals * sizeof(*locals));
    for (i = 0; i < copy->nlocals; i++) {
        if (f->locals[i]) {
            /* copy the object */
            locals[i] = copy_object(f->locals[i]);
        }
    }
    copy->locals = locals;

    return copy;
}

/**
 * Delete a Frame that was copied from another Frame
 *
 * @param f Frame copy to delete
 */
void Frame_delete_copy(Frame *f)
{
    int i;

    if (f == NULL) {
        DIE("%s", "Can't delete a NULL copied frame\n");
    }

    free(f->locals);
    free(f);
}

/**
 * Completely delete a Frame.
 *
 * @param f Frame to delete
 */
void Frame_delete(Frame *f)
{
    if (f->instructions) {
        free(f->instructions);
    }
    if (f->locals) {
        free(f->locals);
    }

    if (f->constants) {
        free(f->constants);
    }

    free(f);
}

/**
 * Delete a Frame in Luci's interactive mode.
 *
 * The Frame's local symbol table and constant table must
 * remain intact.
 *
 * @param f Frame to clean up
 */
void Frame_delete_interactive(Frame *f)
{
    free(f->instructions);
    free(f);
}

/**
 * Creates a new Frame derived from the given CompileState
 *
 * @param cs given CompileState
 * @param nparams number of function parameters for function Frames
 * @returns new Frame
 */
Frame *Frame_from_CompileState(CompileState *cs, uint16_t nparams)
{
    Frame *f = alloc(sizeof(*f));

    f->ip = 0;
    f->nparams = nparams;
    f->ninstrs = cs->instr_count;
    f->instructions = cs->instructions;
    /* get locals object array and size of array */
    if (cs->ltable) {
        f->nlocals = cs->ltable->count;
        f->locals = symtable_get_objects(cs->ltable);
    }
    /* get constants object array and size of array */
    if (cs->ctable) {
        f->nconstants = cs->ctable->count;
        f->constants = cotable_get_objects(cs->ctable);
    }
    /* get globals array. array size doesn't matter because the global
     * objects are never freed in a Frame_delete() call. */
    if (cs->gtable) {
        f->globals = symtable_get_objects(cs->gtable);
    }

    return f;
}

/**
 * Allocates and initializes a new CompileState
 *
 * @returns new, initialized CompileState
 */
CompileState *CompileState_new(void)
{
    CompileState *cs = alloc(sizeof(*cs));
    cs->instr_count = 0;
    cs->instr_alloc = BASE_INSTR_COUNT;
    cs->instructions = alloc(cs->instr_alloc *
            sizeof(*cs->instructions));

    cs->ltable = symtable_new(BASE_SYMTABLE_SCALE);
    cs->ctable = cotable_new(BASE_COTABLE_SIZE);

    cs->current_loop = NULL;

    return cs;
}

/**
 * Deallocates the given CompileState.
 *
 * @param cs given CompileState
 */
void CompileState_delete(CompileState *cs)
{
    /* free(cs->instructions); */ /* Frame owns instructions */
    symtable_delete(cs->ltable);
    cotable_delete(cs->ctable);
    free(cs);
}

/**
 * Allocates a new instructions array for the given CompileState
 *
 * All other members of the CompileState are untouched.
 *
 * @param cs given CompileState
 * @returns modifed CompileState
 */
CompileState *CompileState_refresh(CompileState *cs)
{
    /* set up new instructions pointer */
    cs->instr_count = 0;
    cs->instr_alloc = BASE_INSTR_COUNT;
    cs->instructions = alloc(cs->instr_alloc *
            sizeof(*cs->instructions));

    cs->current_loop = NULL;

    return cs;
}

/**
 * Add a new instruction.
 *
 * @param cs current CompileState
 * @param op opcode for new instruction
 * @param arg argument to given opcode
 * @returns the address of the new instruction
 */
static uint32_t push_instr(CompileState *cs, Opcode op, int arg)
{
    int count = 0;
    uint32_t old_addr;

    if (!cs)
        DIE("%s", "CompileState not allocated. Can't add instruction\n");
    if (!(cs->instructions))
        DIE("%s", "Instruction list not allocated. Can't add instruction\n");

    /* Reallocate the CompileState's instruction list if necessary */
    if (cs->instr_count + 1 > cs->instr_alloc) {
        cs->instr_alloc <<= 1;
        cs->instructions = realloc(cs->instructions,
                cs->instr_alloc * sizeof(*(cs->instructions)));
    }

    count = put_instr(cs, cs->instr_count, op, arg);
    old_addr = cs->instr_count;
    cs->instr_count += count;

    return old_addr;
}

/**
 * Change the instruction at the given address.
 *
 * @param cs current CompileState
 * @param addr address to modify
 * @param op opcode for new instruction
 * @param arg argument to given opcode
 * @returns the number of addresses used for the new instruction
 */
static uint32_t put_instr(CompileState *cs, uint32_t addr,
        Opcode op, int arg) {

    /* if (addr < 0) ... addr is unsigned */
    if (addr > cs->instr_count)
        DIE("%s", "Address out of bounds\n");

    Instruction instr = (op << 11);
    if (op >= JUMP) {
        instr |= (0x7FF & (arg >> 16));
        cs->instructions[addr] = instr;
        instr = 0xFFFF & arg;
        cs->instructions[++addr] = instr;
        return 2;
    } else {
        instr |= (arg & 0x7FF);
        /* Append the new instruction to the instruction list */
        cs->instructions[addr] = instr;
        /* increment instruction count after appending */
        return 1;
    }
}

/**
 * Add a new empty loop_list struct to the current CompileState
 *
 * @param cs CompileState
 * @param loop_type type of the loop (for/while)
 */
static void add_new_loop(CompileState *cs, int loop_type)
{
    struct loop_list *loop = alloc(sizeof(*loop));
    loop->loop_type = loop_type;
    loop->breaks = NULL;
    loop->continues = NULL;
    loop->parent = cs->current_loop;

    cs->current_loop = loop;
}

/**
 * Populate jump addresses for each continue/break in a loop.
 *
 * @param cs CompileState
 * @param start address of first instruction of the loop
 * @param end address of the first instruction after the loop
 */
static void back_patch_loop(CompileState *cs, uint32_t start, uint32_t end)
{
    struct loop_jump *ptr = NULL, *old = NULL;
    struct loop_list *cur_loop = cs->current_loop;
    struct loop_list *parent_loop = cs->current_loop->parent;

    ptr = cur_loop->continues;
    while (ptr) {
        put_instr(cs, ptr->addr, JUMP, start);
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
        put_instr(cs, ptr->addr, break_jump, end);
        old = ptr;
        ptr = ptr->next;
        free(old);
    }

    free(cur_loop);
    cs->current_loop = parent_loop;
}

/**
 * Not Implemented.
 *
 * Serializes a Frame to a string of bytes that can
 * be written to a file.
 *
 * @param globalframe global frame to serialize
 * @returns C-style string of bytes
 */
char* serialize_program(Frame *globalframe)
{
    int i;
    for (i = 0; i < globalframe->nlocals; i++) {
        /* print_object(globalframe->locals[i]); */
    }
    return NULL;
}

/**
 * C-string representations corresponding to each enumerated opcode
 */
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
    "MKMAP",
    "MKLIST",
    "CGET",
    "CPUT",
    "MKITER",
    "HALT",
    /* here begins extended length instructions */
    "JUMP",
    "POPJUMP",
    "JUMPZ",
    "ITERJUMP",
};

/**
 * Prints string representations of each instruction in the CompileState.
 *
 * Used for debugging (or fun)
 *
 * @param f Frame
 */
void print_instructions(Frame *f)
{
    int i, a, instr;
    const char *name = NULL;
    LuciObject *obj;

    for (i = 0; i < f->ninstrs; i ++) {
        a = f->instructions[i] & 0x7FF;
        instr = f->instructions[i] >> 11;

        /* rip out another instr if extended */
        if (instr >= JUMP) {
            a = f->instructions[i + 1] + (a << 16);
        }
        printf("%03x: %s 0x%x\n", i, instruction_names[instr], a);
        /* increment 'i' if we just printed an extended instruction */
        i += (instr >= JUMP) ? 1 : 0;
    }

    for (i = 0; i < f->nlocals; i ++) {
        obj = f->locals[i];
        if (obj && (TYPEOF(f->locals[i]) == obj_func_t)) {
            printf("Symbol 0x%X:\n", i);
            print_instructions(AS_FUNCTION(f->locals[i])->frame);
        }
    }
}
