%{
#include "stdio.h"
#include <iostream>
#include "Memory_Profiler_tab.tab.h"

using namespace std;



extern "C" {

	int yylex(void);
	void yyerror(const char *s);
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
"add"				{return ADD;}
"remove"			{return REMOVE;}
"analyzer"			{return ANALYZER;}
"analyzers"			{return ANALYZERS;}
"pattern"			{return PATTERN;}
"leak"				{return LEAK;}
"dfree"				{return DFREE;}
"alloc"				{return ALLOC;}
"analyze"			{return ANALYZE;}
"filter"			{return FILTER;}
"size"				{return SIZE;}
"all"				{return ALL;}
"exit"  			{return EXIT_COMMAND;}
"help"				{return HELP;}
[0-9]+				{yylval.number=atoi(yytext); return NUMBER;}
[a-zA-Z0-9]*		{yylval.text = yytext; return TEXT;}
[\n]                {return yytext[0];}
[ \t]+              ;
.					{return UNRECOGNIZED_TOKEN;}
%%
