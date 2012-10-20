%option noyywrap

%{

#include <string.h>
#include "parser.tab.h"


static int LINE = 1;
static int COL = 1;

int get_line_num(void);
int get_last_col_num(void);

%}

INTEGER	    [0-9]+
FLOAT	    [0-9]+\.[0-9]*|\.[0-9]+
WORD	    [a-zA-Z_][a-zA-Z_0-9]*
STRING	    \"[^\"]+\"
WS	    [ \t\f\r]+
BAD         [:]
IGNORE      ({WS}|;)+

%%

[wW][hH][iI][lL][eE]	    { COL += yyleng; return WHILE; }
[fF][oO][rR]		    { COL += yyleng; return FOR; }
[iI][nN]		    { COL += yyleng; return IN; }
[dD][oO]		    { COL += yyleng; return DO; }
[dD][oO][nN][eE]	    { COL += yyleng; return DONE; }
[iI][fF]		    { COL += yyleng; return IF; }
[tT][hH][eE][nN]	    { COL += yyleng; return THEN; }
[eE][lL][sS][eE]	    { COL += yyleng; return ELSE; }
[eE][nN][dD]		    { COL += yyleng; return END; }
[dD][eE][fF]                { COL += yyleng; return DEF; }
[rR][eE][tT][uU][rR][nN]    { COL += yyleng; return RETURN; }

^\n|{IGNORE}\n	{ LINE++; COL = 1; /* ignore empty lines */ }
\n	        { LINE++; COL = 1; return NEWLINE; }
#.*\n	        { LINE++; COL = 1; /* ignore one-line comments */ }
{IGNORE}        { COL += yyleng; } /* ignore whitespace/semicolon */

{STRING}	{
                    yylval.string_v = strndup(yytext + 1, strlen(yytext) - 2);
                    COL += yyleng;
                    return STRING;
                }
{FLOAT}		{
                    yylval.float_v = atof(yytext);
                    COL += yyleng;
                    return FLOAT;
                }
{INTEGER}	{
                    yylval.int_v = atoi(yytext);
                    COL += yyleng;
                    return INT;
                }
"True"          {
                    yylval.int_v = 1;
                    COL += yyleng;
                    return INT;
                }
"False"         {
                    yylval.int_v = 0;
                    COL += yyleng;
                    return INT;
                }
{WORD}		{ yylval.string_v = strdup(yytext);
                    COL += yyleng;
                    return ID;
                }

","	{ COL += yyleng; return COMMA; }
"["	{ COL += yyleng; return LSQUARE; }
"]"	{ COL += yyleng; return RSQUARE; }

"-"	{ COL += yyleng; return MINUS; }
"+"	{ COL += yyleng; return PLUS; }
"**"	{ COL += yyleng; return POW; }
"*"	{ COL += yyleng; return TIMES; }
"/"	{ COL += yyleng; return DIVIDE; }
"%"	{ COL += yyleng; return MOD; }

"<="	{ COL += yyleng; return LTHEQ; }
">="	{ COL += yyleng; return GTHEQ; }
"=="	{ COL += yyleng; return EQUAL; }
"!="	{ COL += yyleng; return NOTEQ; }
"<"	{ COL += yyleng; return LTHAN; }
">"	{ COL += yyleng; return GTHAN; }

"!"	{ COL += yyleng; return LGNOT; }
"||"	{ COL += yyleng; return LGOR; }
"&&"	{ COL += yyleng; return LGAND; }

"("	{ COL += yyleng; return LPAREN; }
")"	{ COL += yyleng; return RPAREN; }
"="	{ COL += yyleng; return ASSIGN; }

"^"	{ COL += yyleng; return BWXOR; }
"|"	{ COL += yyleng; return BWOR; }
"&"	{ COL += yyleng; return BWAND; }
"~"	{ COL += yyleng; return BWNOT; }

{BAD}   { COL += yyleng; yyerror("Found invalid char %c", yytext[0]); }

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
