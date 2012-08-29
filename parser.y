%{
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ast.h"
#include "astexec.h"

#define YYDEBUG 1

#define YYPARSE_PARAM root

int VERBOSE;
int LINE_NUM = 1;

void yyerror(const char *msg);

%}

%error-verbose

%union {
    int num;
    char *id;
    ASTNode *node;
}

%start program

%type <node> expr call assignment statement statements program

%token <num> NUM
%token <id> ID
%token NEWLINE
%token EQUALS PRINT
%left PLUS MINUS
%left TIMES DIVIDE
%left LPAREN RPAREN
%left GRTHAN LETHAN
%nonassoc UMINUS

%%

program:    /* nothing */		{ $$ = 0; }
	|   statements			{ (*(ASTNode **)root) = $1; }
	;

statements:
	    statement NEWLINE		    { $$ = make_statement(NULL, $1); }
	|   statements statement NEWLINE    { $$ = make_statement($1, $2); }
	;

statement:
	    assignment			{ $$ = $1; }
	|   call			{ $$ = $1; }
	;

assignment:
	    ID EQUALS expr		{ $$ = make_assignment($1, $3); }
	;

call:
	    ID LPAREN expr RPAREN	{ $$ = make_call($1, $3); }
	;

expr:
	    NUM				{ $$ = make_expr_from_num($1); }
	|   ID				{ $$ = make_expr_from_id($1); }
	|   MINUS expr %prec UMINUS	{ $$ = make_expression(make_expr_from_num(0), $2, '-'); }
	|   expr PLUS expr		{ $$ = make_expression($1, $3, '+'); }
	|   expr MINUS expr		{ $$ = make_expression($1, $3, '-'); }
	|   expr TIMES expr		{ $$ = make_expression($1, $3, '*'); }
	|   expr DIVIDE	expr		{ $$ = make_expression($1, $3, '/'); }
	|   LPAREN expr RPAREN		{ $$ = make_expression(make_expr_from_num(0), $2, '+'); }
	;
%%

void yyerror(const char *msg)
{
    fprintf(stderr, "ERROR: %s @ line #%d\n", msg, LINE_NUM);
    exit(-1);
}

int main()
{
    VERBOSE = 0;
    //yydebug(1);
    ASTNode *root = 0;
    yyparse(&root);
    assert(root);
    struct ExecEnviron *env = create_env();
    exec_AST(env, root);
    destroy_env(env);
    destroy_AST(root);
}
