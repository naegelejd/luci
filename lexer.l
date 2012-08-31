%option noyywrap

%{
#include <string.h>
#include "parser.h"

extern int LINE_NUM;

%}

%x ERROR

NUM	[0-9]+
WORD	[a-zA-Z_][a-zA-Z_0-9]*
WS	[ \t\f\r]+

%%

    /*print	{ return PRINT; } */

{NUM}	{ yylval.num = atoi(yytext); return NUM; }
{WORD}	{ yylval.id = strdup(yytext); return ID; }
{WS}	; /* ignore */

#.*	; /* ignore one-line comments */

^\n	{ LINE_NUM++; /* ignore empty lines */ }
\n	{ LINE_NUM++; return NEWLINE; }

"-"	{ return MINUS; }
"+"	{ return PLUS; }
"*"	{ return TIMES; }
"/"	{ return DIVIDE; }
"<"	{ return LETHAN; }
">"	{ return GRTHAN; }
"("	{ return LPAREN; }
")"	{ return RPAREN; }
"="	{ return EQUALS; }

%%
/*
.	{
	    BEGIN(ERROR);
	    yymore();
	}
<ERROR>[^{NUM}{WORD}+\-/*()= \t\n\f\r] { yymore(); }
<ERROR>(.|\n)	    { 
			yyless(yyleng-1);
			fprintf(stderr, "error token: %s on line %d\n", yytext, LINE_NUM);
			BEGIN(INITIAL);
		    }
*/
