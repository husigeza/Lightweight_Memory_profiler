%{
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>


#include "../Memory_Profiler_class.h"
#include "../Memory_Profiler_analyzer.h"

extern "C" {    
	int yylex(void);
	void yyerror(const char *s);
	int yyparse(void);
	int yywrap(){
	    return 1;
	}		
}


%}

%start line

%union 
{
	int number;
	char *text;
}

%token <number> NUMBER      
%token <text>   TEXT
%token <text>   TIMESTAMP

%token PROCESS ALL ALIVE
%token PROFILED
%token PRINT 
%token ADD REMOVE 
%token ANALYZE
%token PATTERN
%token ANALYZER LEAK DFREE ALLOC TIME FUNCTIONCOUNT SAVE
%token SYMBOLS MAP BT 
%token FILTER SIZE TIME
%token ON OFF
%token HELP
%token EXIT_COMMAND
%token UNRECOGNIZED_TOKEN






%%
line 	 : command 									{cout << ">> ";}
		 | line command								{cout << ">> ";}
		 ;											

command : PRINT PROCESS NUMBER '\n'      			{mem_prof.Print_process($3);}									 
		| PRINT PROCESS ALL '\n' 					{mem_prof.Print_all_processes();}
		| PRINT PROCESS ALIVE '\n'					{mem_prof.Print_alive_processes();}	
		| PRINT PROCESS PROFILED '\n'				{mem_prof.Print_profiled_processes();}
		| PROCESS NUMBER PROFILED ON '\n'			{mem_prof.Add_process_to_profiling($2);}
		| PROCESS NUMBER PROFILED OFF '\n'			{mem_prof.Remove_process_from_profiling($2);}
		| PROCESS ALL PROFILED ON '\n'				{mem_prof.Add_all_process_to_profiling();}
		| PROCESS ALL PROFILED OFF '\n'				{mem_prof.Remove_all_process_from_profiling();}
		| ADD PATTERN TEXT '\n'						{mem_prof.Create_new_pattern($3); free($3);}
		| ADD ANALYZER LEAK '\n'					{mem_prof.Create_new_analyzer(*(new Memory_Leak_Analyzer()));}
		| ADD ANALYZER DFREE '\n'					{mem_prof.Create_new_analyzer(*(new Double_Free_Analyzer()));}
		| ADD ANALYZER PRINT '\n'					{mem_prof.Create_new_analyzer(*(new Print_Analyzer()));}
		| ADD ANALYZER ALLOC '\n'					{mem_prof.Create_new_analyzer(*(new Malloc_Counter_Analyzer()));}
		| ADD ANALYZER SAVE MAP '\n'				{mem_prof.Create_new_analyzer(*(new Save_memory_mappings_Analyzer()));}
		| ADD ANALYZER SAVE SYMBOLS '\n'			{mem_prof.Create_new_analyzer(*(new Save_symbol_table_Analyzer()));}
		| ADD ANALYZER SAVE BT '\n'					{mem_prof.Create_new_analyzer(*(new Save_shared_memory_Analyzer()));}
		| ADD ANALYZER TIME '\n'					{mem_prof.Create_new_analyzer(*(new Average_time_Analyzer()));}
		| ADD ANALYZER FUNCTIONCOUNT '\n'			{mem_prof.Create_new_analyzer(*(new Function_counter_Analyzer()));}
		| ADD FILTER SIZE NUMBER TEXT '\n'			{mem_prof.Create_new_size_filter_cli($4,$5);free($5);}
		| ADD FILTER TIME TIMESTAMP NUMBER TEXT TEXT '\n'	{mem_prof.Create_new_time_filter_cli($4,$5,$6,$7);free($4);free($6);free($7);}	
		| ADD ANALYZER NUMBER PATTERN NUMBER '\n'	{mem_prof.Add_analyzer_to_pattern($3,$5);}
		| ADD ANALYZER NUMBER PATTERN TEXT '\n'		{mem_prof.Add_analyzer_to_pattern_by_name($3,$5);free($5);}
		| REMOVE ANALYZER NUMBER '\n'				{mem_prof.Remove_analyzer($3);}
		| REMOVE ANALYZER NUMBER PATTERN NUMBER '\n'{mem_prof.Remove_analyzer_from_pattern($3,$5);}
		| REMOVE ANALYZER NUMBER PATTERN TEXT '\n'	{mem_prof.Remove_analyzer_from_pattern_by_name($3,$5);free($5);}
		| ADD FILTER NUMBER PATTERN NUMBER '\n'		{mem_prof.Add_filter_to_pattern($3,$5);}
		| ADD FILTER NUMBER PATTERN TEXT '\n'		{mem_prof.Add_filter_to_pattern_by_name($3,$5);free($5);}
		| REMOVE FILTER NUMBER '\n'					{mem_prof.Remove_filter($3);}
		| REMOVE FILTER NUMBER PATTERN NUMBER '\n'	{mem_prof.Remove_filter_from_pattern($3,$5);}
		| REMOVE FILTER NUMBER PATTERN TEXT '\n'	{mem_prof.Remove_filter_from_pattern_by_name($3,$5);free($5);}
		| PROCESS NUMBER ANALYZE PATTERN NUMBER '\n'{mem_prof.Run_pattern($5,$2);}
		| PROCESS NUMBER ANALYZE PATTERN TEXT '\n'	{mem_prof.Run_pattern($5,$2);free($5);}
		| PROCESS ALL ANALYZE PATTERN NUMBER '\n'	{mem_prof.Run_pattern_all_process($5);}
		| PROCESS ALL ANALYZE PATTERN TEXT '\n'		{mem_prof.Run_pattern_all_process($5);free($5);}
		| PRINT ANALYZER ALL '\n'					{mem_prof.Print_analyzers();}
		| PRINT PATTERN ALL '\n'					{mem_prof.Print_patterns();}
		| PRINT FILTER ALL '\n'						{mem_prof.Print_filters();}
		| PRINT PATTERN TEXT '\n'					{mem_prof.Print_pattern($3);}
		| PRINT ANALYZER NUMBER '\n'				{mem_prof.Print_analyzer($3);}
		| PRINT FILTER NUMBER '\n'					{mem_prof.Print_filter($3);}
		| HELP '\n'									{Print_help();}
		| EXIT_COMMAND '\n'    						{exit(1);}
		| '\n'										{;}
		| error '\n'								{cout << "Unrecognized command!" << endl;}
		;
	

%%


#define path_to_FIFO "/dev/mem_prof_fifo"
#define path_to_overload_FIFO "/dev/mem_prof_fifo_overload"

using namespace std;

static Memory_Profiler mem_prof(path_to_FIFO,path_to_overload_FIFO);
static pthread_t FIFO_read_thread_id;
static pthread_t overload_FIFO_read_thread_id;

extern "C" {
	void yyerror(const char *s){
			cout << s << endl;
	}
}
	
void Print_help(){
	
	cout << "HELP:" << endl;
	
	cout << endl << "number = Process's PID" << endl;
	cout << "bt = backtraces of the process" << endl;
	
	cout << endl << "Note for index value: " << endl << endl;
	cout << "add analyzer/filter index to pattern ..." << endl;
	cout <<	"remove analyzer/filter index" << endl;
	cout << "In these cases index refers to the global analyzer/filter container" << endl << endl;
	cout << "remove analyzer/filter index pattern ..." << endl;
	cout << "In this case index refers to the corresponding pattern" << endl << endl;
	cout << "index after pattern always refers to the global pattern index" << endl << endl;
	
	cout <<"Analyzers: " << endl;
	cout <<"Types: "<< endl;
	cout <<"	leak"<< endl;
	cout <<"	print"<< endl;
	cout <<"	dfree"<< endl;
	cout <<"	save symbols"<< endl;
	cout <<"	save map"<< endl;
	cout <<"	save bt"<< endl;
	cout <<"	time" << endl;
	cout <<"    functioncount" << endl << endl;
	
	cout <<"Filters: " << endl;
	cout <<"Size filter: "<< endl;
	cout <<"	type: size" << endl;
	cout <<"	params: [VALUE in BYTES] [equal | less | bigger]" << endl;
	cout <<"Time filter: "<< endl;
	cout <<"	type: time" << endl;
	cout <<"	params: [DATE-TIME] [USEC] [after | before] [equal | less | bigger]" << endl;
	cout <<		"DATE-TIME format: YYYY-MM-DD-HH:MM:SS" << endl << endl; 

	cout << endl << "Recognized commands:" << endl;

	cout <<"print process number" << endl;
	cout <<"print process all" << endl;
	cout <<"print process alive" << endl;
	cout <<"print process profiled" << endl;
	cout <<"print analyzer all" << endl;
	cout <<"print filter all" << endl;
	cout <<"print pattern all" << endl;
	cout <<"print analyzer index" << endl;
	cout <<"print filter index" << endl;
	cout <<"print pattern name" << endl;
	
	cout<< endl;
	
	cout <<"add analyzer type" << endl;
	cout <<"add filter type params" << endl;
	cout <<"add pattern name" << endl;
	cout <<"add analyzer index pattern index" << endl;
	cout <<"add analyzer index pattern name" << endl;
	cout <<"add filter index pattern index" << endl;
	cout <<"add filter index pattern name" << endl;
		
	cout<< endl;
	
	cout <<"remove analyzer index" << endl;
	cout <<"remove analyzer index pattern index" << endl;
	cout <<"remove analyzer index pattern name" << endl;
	cout <<"remove filter index" << endl;
	cout <<"remove filter index pattern index" << endl;
	cout <<"remove filter index pattern name" << endl;
			
	cout<< endl;
	
	cout <<"process number profiled on" << endl;
	cout <<"process number profiled off" << endl;
	cout <<"process all profiled on" << endl;
	cout <<"process all profiled off" << endl;
	
	cout <<"process number analyze pattern index" << endl;
	cout <<"process number analyze pattern name" << endl;
	cout <<"process all analyze pattern index" << endl;
	cout <<"process all analyze pattern name" << endl;
		
	cout<< endl;
	
	cout <<"help" << endl;
	cout <<"exit" << endl;
	cout << endl;

}

void signal_callback_handler(int signum){
	cout << " Signal callback" << endl;

	// Terminate program, will call destructors
	exit(signum);
}


void* Read_FIFO_thread(void *arg) {

	while (true) {
		mem_prof.Read_FIFO();
		usleep(500000);
		
	}	
}

void* Read_overload_FIFO_thread(void *arg) {

	while (true) {
		mem_prof.Read_overload_FIFO();
		usleep(100);
	}	
}


int main() {

	signal(SIGINT, signal_callback_handler);

	int err = pthread_create(&FIFO_read_thread_id, NULL, &Read_FIFO_thread, NULL);
	if (err) {
		cout << "Thread creation failed error: " << err << endl;
	} else {
		//cout << "Read_FIFO_thread created" << endl;
	}
	
	err = pthread_create(&overload_FIFO_read_thread_id, NULL, &Read_overload_FIFO_thread, NULL);
	if (err) {
		cout << "Thread creation failed error: " << err << endl;
	} else {
		//cout << "Read_overload_FIFO_thread created" << endl;
	}
	

	cout << ">> "; 
	return yyparse();
}
