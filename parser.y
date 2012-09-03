%{
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "ast.h"
#include "env.h"

#define YYDEBUG 1

#define YYPARSE_PARAM root

int VERBOSE;
int LINE_NUM = 1;

void yyerror(const char *msg);

%}

%error-verbose

%union {
    int i_val;
    double d_val;
    char *s_val;
    char *id;
    struct ASTNode *node;
}

%start program

%type <node> expr call assignment while statement statements program

%token <i_val> INT
%token <d_val> DOUBLE
%token <s_val> STRING
%token <id> ID
%token NEWLINE
%token WHILE DO DONE

%left LGOR LGAND
%left BWOR BWXOR BWAND
%left EQUAL NOTEQ 
%left LTHAN LTHEQ GTHAN GTHEQ
%left PLUS MINUS
%left TIMES DIVIDE MOD
%left POW
%left LPAREN RPAREN

%right ASSIGN
%right LGNOT BWNOT
%right UPLUS UMINUS ULGNOT UBWNOT

%%

program:    /* nothing */		{ $$ = 0; }
	|   statements			{ (*(struct ASTNode **)root) = $1; }
	;

statements:
	    statement NEWLINE		    { $$ = make_statement(NULL, $1); }
	|   statements statement NEWLINE    { $$ = make_statement($1, $2); }
	;

statement:
	    assignment			{ $$ = $1; }
	|   call			{ $$ = $1; }
	|   while			{ $$ = $1; }
	;

assignment:
	    ID ASSIGN expr		{ $$ = make_assignment($1, $3); }
	;

call:
	    ID LPAREN expr RPAREN	{ $$ = make_call($1, $3); }
	;

while:
	    WHILE expr DO NEWLINE statements DONE	    { $$ = make_while($2, $5); }
	|   WHILE expr NEWLINE DO NEWLINE statements DONE   { $$ = make_while($2, $6); }
	;

expr:
	    INT				{ $$ = make_expr_from_int($1); }
	|   DOUBLE			{ $$ = make_expr_from_double($1); }
	|   STRING			{ $$ = make_expr_from_string($1); }
	|   ID				{ $$ = make_expr_from_id($1); }
	|   MINUS expr %prec UMINUS	{ $$ = make_binary_expr(
						make_expr_from_int(0), $2, op_sub_t); }
	|   expr PLUS expr		{ $$ = make_binary_expr($1, $3, op_add_t); }
	|   expr MINUS expr		{ $$ = make_binary_expr($1, $3, op_sub_t); }
	|   expr TIMES expr		{ $$ = make_binary_expr($1, $3, op_mul_t); }
	|   expr DIVIDE	expr		{ $$ = make_binary_expr($1, $3, op_div_t); }
	|   expr POW expr		{ $$ = make_binary_expr($1, $3, op_pow_t); }
	|   expr MOD expr		{ $$ = make_binary_expr($1, $3, op_mod_t); }
	|   expr LTHAN expr		{ $$ = make_binary_expr($1, $3, op_lt_t); }
	|   expr GTHAN expr		{ $$ = make_binary_expr($1, $3, op_gt_t); }
	|   expr NOTEQ expr		{ $$ = make_binary_expr($1, $3, op_neq_t); }
	|   expr EQUAL expr		{ $$ = make_binary_expr($1, $3, op_eq_t); }
	|   expr LTHEQ expr		{ $$ = make_binary_expr($1, $3, op_lte_t); }
	|   expr GTHEQ expr		{ $$ = make_binary_expr($1, $3, op_gte_t); }
	|   LGNOT expr %prec ULGNOT 	{ $$ = make_binary_expr(
						make_expr_from_int(0), $2, op_lnot_t); }
	|   expr LGOR expr		{ $$ = make_binary_expr($1, $3, op_lor_t); }
	|   expr LGAND expr		{ $$ = make_binary_expr($1, $3, op_land_t); }
	|   expr BWXOR expr		{ $$ = make_binary_expr($1, $3, op_bxor_t); }
	|   expr BWOR expr		{ $$ = make_binary_expr($1, $3, op_bor_t); }
	|   expr BWAND expr		{ $$ = make_binary_expr($1, $3, op_band_t); }
	|   BWNOT expr %prec UBWNOT	{ $$ = make_binary_expr(
						make_expr_from_int(0), $2, op_bnot_t); }
	|   LPAREN expr RPAREN		{ $$ = $2; /*make_binary_expr(
						make_expr_from_int(0), $2, op_add_t);*/ }
	;
%%

void yyerror(const char *msg)
{
    fprintf(stderr, "ERROR: %s @ line #%d\n", msg, LINE_NUM);
    exit(-1);
}

int main(int argc, char *argv[])
{
    --argc, ++argv;

    if (argc > 0)
	VERBOSE = 1;
    else
	VERBOSE = 0;
    //yydebug(1);
    struct ASTNode *root = 0;
    yyparse(&root);
    assert(root);
    struct ExecEnviron *env = create_env();
    exec_AST(env, root);
    destroy_env(env);
    destroy_AST(root);
}
