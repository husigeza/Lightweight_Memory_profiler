%{

#include <stdio.h>

	int yylex(void);
	void yyerror(const char *s);
	int yyparse(void);
	int yywrap(){
	    return 1;
	    }



%}

%start line

%union 
{
	int number;
	char *text;
}


%token <text> TEXT
%token EXIT

%%

line 	 : command 									{printf(">> ");}
		 | line command								{printf(">> ");}
		 ;											

		 
command : TEXT '\n'      			{printf("text: %s\n",$1);free($1);}
		  | TEXT TEXT '\n'           {printf("$1: %s  $2: %s\n",$1,$2);free($1);free($2);}
	      | EXIT {printf("Exiting...\n"); exit(0);}
			;
			
			
%%

void yyerror(const char *s){
		printf("error\n");
	}
	
	
int main() {

	printf(">> "); 
	return yyparse();
}

