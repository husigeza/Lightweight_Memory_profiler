%{

#include "../test.tab.h"

	int yylex(void);
	void yyerror(const char *s);

%}

%%

"exit"				{return EXIT;}
[a-zA-Z0-9]*		{int i; printf("yytext: %s\n",yytext); printf("yyleng: %d\n",yyleng);					
					 yylval.text = strdup(yytext); return TEXT;}
[\n]                {return yytext[0];}
[ \t]+              ;


%%