%{

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "ast.h"
#include "env.h"

#define YYDEBUG 1

#define YYPARSE_PARAM root

int LINE_NUM = 1;

void yyerror(const char *msg);

%}

%error-verbose

%union {
    long i_val;
    double f_val;
    char *s_val;
    char *id;
    struct AstNode *node;
}

%start program

%type <node> expr cond call assignment statement statements
%type <node> while_loop for_loop if_else
%type <node> func_def params
%type <node> empty_list list_items list_index list
%type <node> program

%token <i_val> INT
%token <f_val> FLOAT
%token <s_val> STRING
%token <id> ID
%token NEWLINE COMMA
%token WHILE FOR IN DO DONE
%token IF THEN ELSE END
%token DEF RETURN

%left LGOR LGAND
%left BWOR BWXOR BWAND
%left EQUAL NOTEQ
%left LTHAN LTHEQ GTHAN GTHEQ
%left PLUS MINUS
%left TIMES DIVIDE MOD
%left POW
%left LPAREN RPAREN
%left LSQUARE RSQUARE

%right ASSIGN
%right LGNOT BWNOT
%right UPLUS UMINUS ULGNOT UBWNOT

%%

program:
        /* nothing */       { $$ = 0; }
    |   statements          { (*(struct AstNode **)root) = $1; }
    ;

statements:
        statement NEWLINE
                { $$ = construct_node(ast_stmnts_t, NULL, $1); }
    |   statements statement NEWLINE
                { $$ = construct_node(ast_stmnts_t, $1, $2); }
    ;

statement:
        func_def            { $$ = $1; }
    |   assignment          { $$ = $1; }
    |   call                { $$ = $1; }
    |   while_loop          { $$ = $1; }
    |   for_loop            { $$ = $1; }
    |   if_else             { $$ = $1; }
    ;

assignment:
        ID ASSIGN expr
                { $$ = construct_node(ast_assign_t, $1, $3); }
    |   ID LSQUARE expr RSQUARE ASSIGN expr
                { $$ = construct_node(ast_listassign_t, $1, $3, $6); }
    ;

func_def:
        DEF ID LPAREN empty_list RPAREN NEWLINE statements END
            { $$ = construct_node(ast_func_t, $2, $4, $7, NULL); }
    |   DEF ID LPAREN empty_list RPAREN NEWLINE RETURN expr NEWLINE END
            { $$ = construct_node(
                    ast_func_t, $2, $4,
                    construct_node(ast_stmnts_t, NULL, NULL), $8
                    );
            }
    |   DEF ID LPAREN empty_list RPAREN NEWLINE statements RETURN expr NEWLINE END
            { $$ = construct_node(ast_func_t, $2, $4, $7, $9); }
    |   DEF ID LPAREN params RPAREN NEWLINE statements END
            { $$ = construct_node(ast_func_t, $2, $4, $7, NULL); }
    |   DEF ID LPAREN params RPAREN NEWLINE RETURN expr NEWLINE END
            { $$ = construct_node(
                    ast_func_t, $2, $4,
                    construct_node(ast_stmnts_t, NULL, NULL), $8
                    );
            }
    |   DEF ID LPAREN params RPAREN NEWLINE statements RETURN expr NEWLINE END
            { $$ = construct_node(ast_func_t, $2, $4, $7, $9); }
    ;

params:
        ID      { $$ = construct_node(
                        ast_list_t, NULL,
                        construct_node(ast_string_t, $1)
                        );
                }
    |   params COMMA ID
                { $$ = construct_node(
                        ast_list_t, $1,
                        construct_node(ast_string_t, $3)
                        );
                }
    ;

call:
        ID LPAREN empty_list RPAREN
                { $$ = construct_node(ast_call_t, $1, $3); }
    |   ID LPAREN list_items RPAREN
                { $$ = construct_node(ast_call_t, $1, $3); }
    ;

list:
        LSQUARE empty_list RSQUARE      { $$ = $2; }
    |   LSQUARE list_items RSQUARE      { $$ = $2; }
    ;

empty_list:
    /* nothing */   { $$ = construct_node(ast_list_t, NULL, NULL); }
    ;

list_items:
        expr        { $$ = construct_node(ast_list_t, NULL, $1); }
    |   list_items COMMA expr   { $$ = construct_node(ast_list_t, $1, $3); }
    ;

list_index:
        expr LSQUARE expr RSQUARE
                { $$ = construct_node(ast_listindex_t, $1, $3); }
    ;

while_loop:
        WHILE cond DO NEWLINE statements DONE
                { $$ = construct_node(ast_while_t, $2, $5); }
    ;

for_loop:
        FOR ID IN expr DO NEWLINE statements DONE
                { $$ = construct_node(ast_for_t, $2, $4, $7); }
    ;

if_else:
        IF cond THEN NEWLINE statements END
                { $$ = construct_node(ast_if_t, $2, $5, NULL); }
    |   IF cond THEN NEWLINE statements ELSE NEWLINE statements END
                { $$ = construct_node(ast_if_t, $2, $5, $8); }
    ;

cond:
        expr                { $$ = $1; }
    |   expr NEWLINE        { $$ = $1; }
    ;

expr:
        INT                     { $$ = construct_node(ast_int_t, $1); }
    |   FLOAT                   { $$ = construct_node(ast_float_t, $1); }
    |   STRING                  { $$ = construct_node(ast_string_t, $1); }
    |   ID                      { $$ = construct_node(ast_id_t, $1); }
    |   expr PLUS expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_add_t); }
    |   expr MINUS expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_sub_t); }
    |   expr TIMES expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_mul_t); }
    |   expr DIVIDE expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_div_t); }
    |   expr POW expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_pow_t); }
    |   expr MOD expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_mod_t); }
    |   expr LTHAN expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_lt_t); }
    |   expr GTHAN expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_gt_t); }
    |   expr NOTEQ expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_neq_t); }
    |   expr EQUAL expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_eq_t); }
    |   expr LTHEQ expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_lte_t); }
    |   expr GTHEQ expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_gte_t); }
    |   expr LGOR expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_lor_t); }
    |   expr LGAND expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_land_t); }
    |   expr BWXOR expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_bxor_t); }
    |   expr BWOR expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_bor_t); }
    |   expr BWAND expr
                { $$ = construct_node(ast_expr_t, $1, $3, op_band_t); }
    |   BWNOT expr %prec UBWNOT
                { $$ = construct_node(ast_expr_t,
                        construct_node(ast_int_t, 0), $2, op_bnot_t
                        );
                }
    |   LGNOT expr %prec ULGNOT
                { $$ = construct_node(ast_expr_t,
                        construct_node(ast_int_t, 0), $2, op_lnot_t
                        );
                }
    |   MINUS expr %prec UMINUS
                { $$ = construct_node(ast_expr_t,
                        construct_node(ast_int_t, 0), $2, op_sub_t
                        );
                }
    |   LPAREN expr RPAREN      { $$ = $2; }
    |   list_index              { $$ = $1; }
    |   list                    { $$ = $1; }
    |   call                    { $$ = $1; }
    ;

%%

void yyerror(const char *msg)
{
    fprintf(stderr, "Syntax Error: %s @ line #%d\n", msg, LINE_NUM);
    /* exit(-1); */
}

