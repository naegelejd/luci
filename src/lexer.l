/*
 * See Copyright Notice in luci.h
 */

%option noyywrap

%{

#include <string.h>
#include "parser.tab.h"


static int LINE = 1;
static int COL = 1;

int get_line_num(void);
int get_last_col_num(void);

%}

INTEGER     [0-9]+
FLOAT       [0-9]+\.[0-9]*|\.[0-9]+
WORD        [a-zA-Z_][a-zA-Z_0-9]*
STRING      \"[^\"]+\"|\"\"
WS          [ \t\f\r]+

%%

"/*"            { comment(); }

"while"         { COL += yyleng; return WHILE; }
"for"           { COL += yyleng; return FOR; }
"in"            { COL += yyleng; return IN; }
"do"            { COL += yyleng; return DO; }
"done"          { COL += yyleng; return DONE; }
"if"            { COL += yyleng; return IF; }
"then"          { COL += yyleng; return THEN; }
"else"          { COL += yyleng; return ELSE; }
"end"           { COL += yyleng; return END; }
"def"           { COL += yyleng; return DEF; }
"break"         { COL += yyleng; return BREAK; }
"continue"      { COL += yyleng; return CONTINUE; }
"return"        { COL += yyleng; return RETURN; }
"pass"          { COL += yyleng; return PASS; }

{STRING}        {
                    yylval.string_v = strndup(yytext + 1, strlen(yytext) - 2);
                    COL += yyleng;
                    return STRING;
                }
{FLOAT}         {
                    yylval.float_v = atof(yytext);
                    COL += yyleng;
                    return FLOAT;
                }
{INTEGER}       {
                    yylval.int_v = atoi(yytext);
                    COL += yyleng;
                    return INT;
                }
"true"          {
                    yylval.int_v = 1;
                    COL += yyleng;
                    return INT;
                }
"false"         {
                    yylval.int_v = 0;
                    COL += yyleng;
                    return INT;
                }
{WORD}          { yylval.string_v = strdup(yytext);
                    COL += yyleng;
                    return ID;
                }

^\n|{WS}+\n     { LINE++; COL = 1; /* ignore empty lines */ }
\n              { LINE++; COL = 1; }
#.*\n           { LINE++; COL = 1; /* ignore one-line comments */ }
{WS}+           { COL += yyleng; } /* ignore whitespace/semicolon */

"{"     { COL += 1; return LBRACK; }
"}"     { COL += 1; return RBRACK; }
";"     { COL += 1; return SEMICOLON; }
","     { COL += 1; return COMMA; }
"["     { COL += 1; return LSQUARE; }
"]"     { COL += 1; return RSQUARE; }

"-"     { COL += 1; return MINUS; }
"+"     { COL += 1; return PLUS; }
"**"    { COL += 2; return POW; }
"*"     { COL += 1; return TIMES; }
"/"     { COL += 1; return DIVIDE; }
"%"     { COL += 1; return MOD; }

"<="    { COL += 2; return LTHEQ; }
">="    { COL += 2; return GTHEQ; }
"=="    { COL += 2; return EQUAL; }
"!="    { COL += 2; return NOTEQ; }
"<"     { COL += 1; return LTHAN; }
">"     { COL += 1; return GTHAN; }

"!"     { COL += 1; return LGNOT; }
"||"    { COL += 2; return LGOR; }
"&&"    { COL += 2; return LGAND; }

"("     { COL += 1; return LPAREN; }
")"     { COL += 1; return RPAREN; }
"="     { COL += 1; return ASSIGN; }

"^"     { COL += 1; return BWXOR; }
"|"     { COL += 1; return BWOR; }
"&"     { COL += 1; return BWAND; }
"~"     { COL += 1; return BWNOT; }

.       { COL += yyleng; /* ignore invalid chars */ }

<<EOF>>     {
                yy_delete_buffer(YY_CURRENT_BUFFER);
                yyterminate();
            }


%%

int get_line_num(void)
{
    return LINE;
}

int get_last_col_num(void)
{
    return COL - yyleng;
}

comment()
{
    char c, c1;

loop:
    while ((c = input()) != '*' && c != 0) {
        ;
    }

    if ((c1 = input()) != '/' && c != 0) {
        unput(c1);
        goto loop;
    }

}

