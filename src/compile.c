/*
 * See Copyright Notice in luci.h
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

static void add_new_loop(CompileState *cs, uint8_t loop_type);
static void back_patch_loop(CompileState *cs, uint32_t start, uint32_t end);



static void compile_int_constant(AstNode *node, CompileState *cs)
{
    int a;
    LuciObject *obj;

    obj = LuciInt_new(node->data.i);
    a = constant_id(cs->ctable, obj);
    push_instr(cs, LOADK, a);
}


static void compile_float_constant(AstNode *node, CompileState *cs)
{
    int a;
    LuciObject *obj;

    obj = LuciFloat_new(node->data.f);
    a = constant_id(cs->ctable, obj);
    push_instr(cs, LOADK, a);
}


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


static void compile_id_expr(AstNode *node, CompileState *cs)
{
    int a;
    a = symtable_id(cs->ltable, node->data.id.val, SYMFIND);
    if (a < 0) {
        a = symtable_id(cs->gtable, node->data.id.val, SYMFIND);
        if (a < 0) {
            DIE("%s undefined.\n", node->data.id.val);
        }
        /* else */
        push_instr(cs, LOADG, a);
    } else {
        push_instr(cs, LOADS, a);
    }
}


static void compile_binary_expr(AstNode *node, CompileState *cs)
{
    int a;
    compile(node->data.expression.left, cs);
    compile(node->data.expression.right, cs);
    a = node->data.expression.op;
    push_instr(cs, BINOP, a);
}


static void compile_list_access(AstNode *node, CompileState *cs)
{
    compile(node->data.listaccess.index, cs);
    compile(node->data.listaccess.list, cs);
    push_instr(cs, LISTGET, 0);
}


static void compile_list_assignment(AstNode *node, CompileState *cs)
{
    compile(node->data.listassign.right, cs);
    compile(node->data.listassign.index, cs);
    compile(node->data.listassign.list, cs);
    push_instr(cs, LISTPUT, 0);
}


static void compile_list_def(AstNode *node, CompileState *cs)
{
    int i;
    /* compile list items in reverse order */
    for (i = node->data.list.count - 1; i >= 0; i--) {
        /* compile each list member */
        compile(node->data.list.items[i], cs);
    }
    /* add MKLIST instruction for # of list items */
    push_instr(cs, MKLIST, node->data.list.count);
}


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


static void compile_for_loop(AstNode *node, CompileState *cs)
{
    uint32_t addr1, addr2;
    int a;

    add_new_loop(cs, LOOP_TYPE_FOR);
    /* compile loop (expr) */
    compile(node->data.for_loop.list, cs);
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


static void compile_func_call(AstNode *node, CompileState *cs)
{
    int i;
    /* compile arglist, which pushes each arg onto stack */
    AstNode *tmp = node->data.call.arglist;
    for (i = 0; i < tmp->data.list.count; i++) {
        compile(tmp->data.list.items[i], cs);
    }
    /* compile funcname, which pushes symbol value onto stack */
    compile(node->data.call.funcname, cs);
    /* add CALL instr, specifying # of args */
    push_instr(cs, CALL, tmp->data.list.count);
}


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

    nparams = params->data.list.count;
    /* add each parameter to symbol table */
    for (i = 0; i < nparams; i++) {
        id_string = params->data.list.items[i];
        /* add arg symbol to symbol table */
        a = symtable_id(func_cs->ltable, id_string->data.s, SYMCREATE);
        //push_instr(func_cs, STORE, a);
    }
    compile(node->data.funcdef.statements, func_cs);

    /* add a RETURN to end of function if necessary */
    if (func_cs->instructions[func_cs->instr_count - 1] != RETURN) {
        push_instr(func_cs, PUSHNULL, 0);
        push_instr(func_cs, RETURN, 0);
    }

    /* create function object */
    obj = LuciFunction_new(Frame_from_CompileState(func_cs, nparams));
    //obj = create_object(obj_func_t);
    //obj->value.func.frame = Frame_from_CompileState(func_cs, nparams);
    //obj->value.func.deleter = free;

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


/* compile all statements in 3 passes :'(
 * 1. put all function names in symbol table
 * 2. compile all global statements
 * 3. compile all statements inside of function defs
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


static void compile_break(AstNode *node, CompileState *cs)
{
    struct loop_jump *jmp = alloc(sizeof(*jmp));
    struct loop_jump *ptr;

    if (!cs->current_loop) {
        DIE("%s", "'break' @ line %d not inside a loop\n", node->lineno);
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


static void compile_continue(AstNode *node, CompileState *cs)
{
    struct loop_jump *jmp = alloc(sizeof(*jmp));
    struct loop_jump *ptr;
    if (!cs->current_loop) {
        DIE("%s", "'continue' @ line %d not inside a loop\n", node->lineno);
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


static void compile_return(AstNode *node, CompileState *cs)
{
    if (node->data.return_stmt.expr == NULL) {
        push_instr(cs, PUSHNULL, 0);
    } else {
        compile(node->data.return_stmt.expr, cs);
    }
    push_instr(cs, RETURN, 0);
}


static void compile_pass(AstNode *node, CompileState *cs)
{
    push_instr(cs, NOP, 0);
}


static void (*compilers[])(AstNode *, CompileState *) = {
    compile_int_constant,
    compile_float_constant,
    compile_string_constant,
    compile_id_expr,
    compile_binary_expr,
    compile_list_access,
    compile_list_assignment,
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


static void compile(AstNode *node, CompileState *cs)
{
    compilers[node->type](node, cs);
}

/**
 * Public entry point for compilation.
 *
 * Allocates a CompileState struct and passes it to
 * the AST walker/compiler.
 */
CompileState * compile_ast(AstNode *root)
{
    int i, id;
    LuciObject *obj;
    CompileState *cs;

    if (!root) {
        DIE("%s", "Nothing to compile\n");
    }

    cs = CompileState_new();

    /* Add builtin symbols to symbol table */
    extern struct func_def builtins[];  /* defined in builtins.c */

    for (i = 0; builtins[i].name != 0; i++) {
        /* create object for builtin function */
        obj = LuciLibFunc_new(builtins[i].func);
        //obj = create_object(obj_libfunc_t);
        //obj->value.libfunc = builtins[i].func;
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

    /* compile the AST */
    compile(root, cs);

    /* end the CompileState with an HALT instr */
    push_instr(cs, HALT, 0);

    return cs;
}

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
            /* copy the refcount. this is important because objects
             * that have a refcount > 1 should not be deleted on calls
             * to "destroy(obj)"
             */
            locals[i]->refcount = f->locals[i]->refcount;
        }
    }
    copy->locals = locals;

    return copy;
}

void Frame_delete_copy(Frame *f)
{
    int i;

    if (f == NULL) {
        DIE("%s", "Can't delete a NULL copied frame\n");
    }

    for (i = 0; i < f->nlocals; i++) {
        decref(f->locals[i]);
    }
    free(f->locals);
    free(f);
}

void Frame_delete(Frame *f)
{
    if (f->instructions) {
        free(f->instructions);
    }
    if (f->locals) {
        int i;
        for (i = 0; i < f->nlocals; i++) {
            /* need to force destruction */
            decref(f->locals[i]);
        }
        free(f->locals);
    }

    if (f->constants) {
        int i;
        for (i = 0; i < f->nconstants; i++) {
            /* need to force destruction */
            destroy(f->constants[i]);
        }
        free(f->constants);
    }

    free(f);
}

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
    /* get globals array. size doesn't matter because it is never
     * freed by a Frame_delete() call. */
    f->globals = symtable_get_objects(cs->gtable);

    return f;
}

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

void CompileState_delete(CompileState *cs)
{
    /* free(cs->instructions); */ /* Frame owns instructions */
    symtable_delete(cs->ltable);
    cotable_delete(cs->ctable);
    free(cs);
}

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

static uint32_t put_instr(CompileState *cs, uint32_t addr,
        Opcode op, int arg) {

    if (addr < 0 || addr > cs->instr_count)
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

static void add_new_loop(CompileState *cs, uint8_t loop_type)
{
    struct loop_list *loop = alloc(sizeof(*loop));
    loop->loop_type = loop_type;
    loop->breaks = NULL;
    loop->continues = NULL;
    loop->parent = cs->current_loop;

    cs->current_loop = loop;
}

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

char* serialize_program(Frame *globalframe)
{
    int i;
    for (i = 0; i < globalframe->nlocals; i++) {
        //print_object(globalframe->locals[i]);
    }
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
 * Prints string representations of each instruction in the CompileState.
 * Used for debugging (or fun)
 */
void print_instructions(Frame *f)
{
    int i, a, instr;
    const char *name = NULL;

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
}
