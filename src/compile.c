/*
 * See Copyright Notice in luci.h
 */

/**
 * @file compile.c
 */

#include "luci.h"
#include "compile.h"
#include "ast.h"
#include "lucitypes.h"
#include "symbol.h"
#include "builtin.h"

/** global symbol table for builtin functions */
SymbolTable *builtin_symbols;
/** global builtins array (from final builtins symbol table) */
LuciObject **builtins;


static void compile(AstNode *, CompileState *);

static uint32_t push_instr(CompileState *, Opcode, int);
static uint32_t put_instr(CompileState *, uint32_t, Opcode, int);

static void add_new_loop(CompileState *cs, int loop_type);
static void back_patch_loop(CompileState *cs, uint32_t start, uint32_t end);


/**
 * Compile a @code nil @endcode AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_nil(AstNode *node, CompileState *cs)
{
    push_instr(cs, PUSHNIL, 0);
}

/**
 * Compile an integer constant AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_int_constant(AstNode *node, CompileState *cs)
{
    int a;
    LuciObject *obj = LuciInt_new(node->data.i);
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
    LuciObject *obj = LuciFloat_new(node->data.f);
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
    LuciObject *obj = LuciString_new(strdup(node->data.s));
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
        return;
    }

    /* didn't find symbol in the locals symbol table */
    if (cs->gtable) {
        /* search the globals table for the symbol */
        a = symtable_id(cs->gtable, node->data.id.val, SYMFIND);
        if (a >= 0) {
            /* found the symbol in the globals table */

            push_instr(cs, LOADG, a);
            return;
        }
    }

    /* didn't find symbol in the globals table either */
    a = symtable_id(builtin_symbols, node->data.id.val, SYMFIND);
    if (a >= 0) {
        /* the symbol is one of Luci's builtin symbols */
        push_instr(cs, LOADB, a);
        return;
    }

    /* symbol not found */
    LUCI_DIE("%s undefined.\n", node->data.id.val);
}


/**
 * Compile a unary expression AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_unary_expr(AstNode *node, CompileState *cs)
{
    compile(node->data.unexpr.right, cs);
    /* offset by opcode 'NEG', which is the first binary opcode */
    push_instr(cs, NEG + (node->data.unexpr.op - op_neg_t), 0);
}

/**
 * Compile a binary expression AST Node
 *
 * @param node AST Node to compile
 * @param cs CompileState to compile to
 */
static void compile_binary_expr(AstNode *node, CompileState *cs)
{
    compile(node->data.binexpr.left, cs);
    compile(node->data.binexpr.right, cs);
    /* offset by opcode 'ADD', which is the first binary opcode */
    push_instr(cs, ADD + node->data.binexpr.op, 0);
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
    /* change bogus jump to a ITERJUMP (get iter->next or jump) */
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
    int a, nparams;
    AstNode *params = node->data.funcdef.param_list;
    AstNode *id_string = NULL;

    /* store empty function object in symbol table */
    LuciObject *obj = LuciFunction_new();
    a = symtable_id(cs->ltable, node->data.funcdef.funcname, SYMCREATE);
    symtable_set(cs->ltable, obj, a);

    /* Create new frame for function scope */
    CompileState *func_cs = compile_state_new();

    /* create globals table for new frame */
    /* !NOTE: in order to support nested function definitions,
     * this globals table needs to include BOTH the parent function's
     * locals table and globals table */
    func_cs->gtable = cs->ltable;

    nparams = params->data.listdef.count;
    /* add each parameter to symbol table */
    int i;
    for (i = 0; i < nparams; i++) {
        id_string = params->data.listdef.items[i];
        /* add arg symbol to symbol table */
        a = symtable_id(func_cs->ltable, id_string->data.s, SYMCREATE);
    }

    /* ensure that the function ends with RETURN instruction */
    AstNode *statements = node->data.funcdef.statements;
    compile(statements, func_cs);
    AstNode *last = statements->data.statements.statements[
            statements->data.statements.count - 1];
    if (last->type != ast_return_t) {
        push_instr(func_cs, PUSHNIL, 0);
        push_instr(func_cs, RETURN, 0);
    }

    /* create function object */
    convert_to_function(func_cs, obj, nparams);

    /* Clean up CompileState created to compile this function */
    compile_state_delete(func_cs);
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
            symtable_id(cs->ltable, tmp->data.funcdef.funcname, SYMCREATE);
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
            if (    (tmp->type == ast_unexpr_t) ||
                    (tmp->type == ast_binexpr_t) ||
                    (tmp->type == ast_call_t) ||
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
    Loopjump *jmp = alloc(sizeof(*jmp));
    Loopjump *ptr;

    if (!cs->current_loop) {
        LUCI_DIE("'break' @ line %d not inside a loop\n", node->lineno);
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
    Loopjump *jmp = alloc(sizeof(*jmp));
    Loopjump *ptr;
    if (!cs->current_loop) {
        LUCI_DIE("'continue' @ line %d not inside a loop\n", node->lineno);
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
        push_instr(cs, PUSHNIL, 0);
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
    compile_nil,
    compile_int_constant,
    compile_float_constant,
    compile_string_constant,
    compile_id_expr,
    compile_unary_expr,
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
 * Initializes the compiler
 *
 * Allocates the global builtins symbol table, populates it,
 * then stores the resulting array of objects in the global
 * builtins array
 */
void compiler_init(void)
{
    /* allocate the global builtin symbol table */
    builtin_symbols = symtable_new(BASE_SYMTABLE_SCALE);

    /* initialize all of Luci's builtin static objects */
    init_luci_builtins();

    /* Add each builtin symbol to builtin symbol table */
    int i;
    for (i = 0; builtins_registry[i].name != NULL; i++) {
        /* add the symbol and function object to the builtins symbol table */
        int id = symtable_id(builtin_symbols,
                builtins_registry[i].name, SYMCREATE);
        symtable_set(builtin_symbols, builtins_registry[i].object, id);
    }

    /* initialize the global builtins array for the interpreter */
    builtins = symtable_copy_objects(builtin_symbols);
}

/**
 * Deletes the global builtins symbol table
 */
void compiler_finalize(void)
{
    symtable_delete(builtin_symbols);
}

/**
 * Public entry point for compilation.
 *
 * Allocates a CompileState struct and passes it to
 * the AST walker/compiler.
 *
 * @param root top-level AST Node
 * @returns complete CompileState
 */
CompileState * compile_ast(AstNode *root)
{
    if (!root) {
        LUCI_DIE("%s", "Nothing to compile\n");
    }

    CompileState *cs = compile_state_new();

    /* compile the AST */
    compile(root, cs);

    /* end the CompileState with a HALT instr */
    push_instr(cs, HALT, 0);

    return cs;
}

/**
 * Public entry point for incremental compilation.
 *
 * Either allocates a new CompileState struct or updates
 * an existing one in the case of incremental compilation
 *
 * @param cs given CompileState
 * @param gf LuciFunctionObj from which to reinstantiate the CompileState
 * @param root top-level AST Node
 * @returns complete CompileState
 */
CompileState *compile_ast_incremental(CompileState *cs, LuciObject *gf, AstNode *root)
{
    /* if we're compiling an AST from scratch, create a new
     * CompileState to pass around */
    if (cs == NULL) {
        cs = compile_state_new();
    } else {
        /* otherwise, reset the CompileState, maintaining the symbol
         * and constant tables */
        cs->instr_count = 0;
        cs->instr_alloc = BASE_INSTR_COUNT;
        cs->instructions = alloc(cs->instr_alloc *
                sizeof(*cs->instructions));

        cs->current_loop = NULL;
    }

    if (gf) {
        LuciFunctionObj *f = AS_FUNCTION(gf);
        memcpy(cs->ltable->objects, f->locals, f->nlocals * sizeof(*f->locals));
    }

    /* compile the AST */
    compile(root, cs);

    /* end the CompileState with a HALT instr */
    push_instr(cs, HALT, 0);

    return cs;
}

/**
 * Creates a new function derived from the given CompileState
 *
 * @param cs given CompileState
 * @param o LuciFunctionObj to populate during conversion
 * @param nparams number of function parameters for resulting function
 * @returns new LuciFunctionObj
 */
void convert_to_function(CompileState *cs, LuciObject *o, uint16_t nparams)
{
    LuciFunctionObj *f = AS_FUNCTION(o);

    f->nparams = nparams;

    /* copy instructions array */
    f->ninstrs = cs->instr_count;
    size_t instr_bytes = f->ninstrs * sizeof(*cs->instructions);
    f->instructions = alloc(instr_bytes);
    memcpy(f->instructions, cs->instructions, instr_bytes);

    f->ip = f->instructions;

    /* get copy of locals object array and size of array */
    f->nlocals = cs->ltable->count;
    /* f->locals = symtable_copy_objects(cs->ltable); */
    f->locals = symtable_give_objects(cs->ltable);

    /* get copy of constants object array and size of array */
    f->nconstants = cs->ctable->count;
    f->constants = cotable_copy_objects(cs->ctable);

    /* get globals array.
     * the array is already owned by a parent function, so never needs
     * freed by this function */
    if (cs->gtable) {
        f->globals = symtable_give_objects(cs->gtable);
    }
}

/**
 * Allocates and initializes a new CompileState
 *
 * @returns new, initialized CompileState
 */
CompileState *compile_state_new(void)
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
void compile_state_delete(CompileState *cs)
{
    free(cs->instructions);
    symtable_delete(cs->ltable);
    cotable_delete(cs->ctable);
    free(cs);
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
    if (!cs) {
        LUCI_DIE("%s", "CompileState not allocated. Can't add instruction\n");
    }
    if (!(cs->instructions)) {
        LUCI_DIE("%s", "Instruction list not allocated. Can't add instruction\n");
    }

    /* Reallocate the CompileState's instruction list if necessary */
    if (cs->instr_count + 1 > cs->instr_alloc) {
        cs->instr_alloc <<= 1;
        cs->instructions = realloc(cs->instructions,
                cs->instr_alloc * sizeof(*(cs->instructions)));
    }

    uint32_t count = put_instr(cs, cs->instr_count, op, arg);
    uint32_t old_addr = cs->instr_count;
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
        Opcode op, int arg)
{
    if (addr > cs->instr_count) {
        LUCI_DIE("%s", "Address out of bounds\n");
    }

    /* JUMP operations must be made relative */
    if (op >= JUMP) {
        arg -= addr;
    }

    /* initialize instruction with the opcode */
    Instruction instr = (op << OPCODE_SHIFT);

    /* add signed bit if the argument is negative */
    if (arg < 0) {
        arg = -arg;
        instr |= OPARG_NEG_BIT;
    }

    instr |= (OPARG_MASK & arg);
    cs->instructions[addr] = instr;

    return 1;   /* number of instructions written at addr */
}

/**
 * Add a new empty Looplist struct to the current CompileState
 *
 * @param cs CompileState
 * @param loop_type type of the loop (for/while)
 */
static void add_new_loop(CompileState *cs, int loop_type)
{
    Looplist *loop = alloc(sizeof(*loop));
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
    Loopjump *ptr = NULL, *old = NULL;
    Looplist *cur_loop = cs->current_loop;
    Looplist *parent_loop = cs->current_loop->parent;

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
 * Serializes a LuciFunctionObj to a string of bytes that can
 * be written to a file.
 *
 * @param globalframe global frame to serialize
 * @returns C-style string of bytes
 */
char* serialize_program(LuciObject *globalframe)
{
    int i;
    for (i = 0; i < AS_FUNCTION(globalframe)->nlocals; i++) {
        /* print_object(globalframe->locals[i]); */
    }
    return NULL;
}

/**
 * C-string representations corresponding to each enumerated opcode
 */
static char *instruction_names[] = {
    "NOP",

    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "POW",
    "EQ",
    "NEQ",
    "LT",
    "GT",
    "LTE",
    "GTE",
    "LGOR",
    "LGAND",
    "BWXOR",
    "BWOR",
    "BWAND",

    "NEG",
    "LGNOT",
    "BWNOT",

    "POP",
    "PUSHNIL",
    "LOADK",
    "LOADS",
    "LOADG",
    "LOADB",
    "DUP",
    "STORE",
    "CALL",
    "RETURN",
    "MKMAP",
    "MKLIST",
    "CGET",
    "CPUT",
    "MKITER",
    "HALT",
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
 * @param o LuciFunctionObj
 */
void print_instructions(LuciObject *o)
{
    LuciFunctionObj *f = AS_FUNCTION(o);

    int i;
    for (i = 0; i < f->ninstrs; i ++) {
        int a = OPARG(f->instructions[i]);
        Instruction instr = OPCODE(f->instructions[i]);
        printf("%03x: %s %d\n", i, instruction_names[instr], a);
    }

    for (i = 0; i < f->nlocals; i ++) {
        LuciObject *obj = f->locals[i];
        if (obj && (ISTYPE(f->locals[i], obj_func_t))) {
            printf("Symbol 0x%X:\n", i);
            print_instructions(f->locals[i]);
        }
    }
}
