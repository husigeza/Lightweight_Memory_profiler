%{
#include "stdio.h"
#include "Memory_Profiler_tab.tab.h"

//#define YYSTYPE double

extern "C" {
	int yylex(void);
	void yyerror(const char *s);
	//extern YYSTYPE yylval;
}

%}

%%
"print"				{return PRINT;}
"alive"				{return ALIVE;}
"profiled"			{return PROFILED;}
"process" 			{return PROCESS;}
"save"				{return SAVE;}
"symbols"			{return SYMBOLS;}
"map"				{return MAP;}
"bt"				{return BT;}
"on"				{return ON;}
"off"				{return OFF;}
"analyze"			{return ANALYZE;}
[0-9]+				{yylval.number=atoi(yytext); return NUMBER;}
"all"				{return ALL;}
"exit"  			{return EXIT_COMMAND;}
"help"				{return HELP;}
[\n]                {return yytext[0];}
[ \t]+              ;
.					{return UNRECOGNIZED_TOKEN;}
%%
