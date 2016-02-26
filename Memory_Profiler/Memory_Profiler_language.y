%{

#include "../Memory_Profiler_class.h"

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <memory>

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

%token <text> PRINT
%token <text> PROCESS 
%token <text> ALL 
%token ALIVE PROFILED BT 
%token ANALYZE ADD REMOVE ANALYZER ANALYZERS PATTERN FILTER SIZE
%token LEAK DFREE
%token SAVE SYMBOLS MAP
%token ON OFF
%token <number> NUMBER
%token <text> TEXT
%token PROMPT
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
		| PRINT PROCESS NUMBER BT '\n'				{mem_prof.Print_process_shared_memory($3);}
		| PRINT PROCESS ALL BT '\n'					{mem_prof.Print_all_processes_shared_memory();}
		| SAVE PROCESS NUMBER SYMBOLS '\n'			{mem_prof.Save_process_symbol_table_to_file($3);}
		| SAVE PROCESS NUMBER MAP '\n'				{mem_prof.Save_process_memory_mapping_to_file($3);}
		| SAVE PROCESS NUMBER BT '\n'				{mem_prof.Save_process_shared_memory_to_file($3);}
		| SAVE PROCESS ALL BT '\n'				    {mem_prof.Save_all_process_shared_memory_to_file();}
		| PROCESS NUMBER PROFILED ON '\n'			{mem_prof.Add_process_to_profiling($2);}
		| PROCESS NUMBER PROFILED OFF '\n'			{mem_prof.Remove_process_from_profiling($2);}
		| PROCESS ALL PROFILED ON '\n'				{mem_prof.Add_all_process_to_profiling();}
		| PROCESS ALL PROFILED OFF '\n'				{mem_prof.Remove_all_process_from_profiling();}
		| ADD PATTERN TEXT '\n'						{mem_prof.Create_new_pattern($3);}
		| ADD ANALYZER LEAK '\n'					{mem_prof.Create_new_analyzer(unique_ptr<Memory_Leak_Analyzer> (new Memory_Leak_Analyzer()));}
		| ADD ANALYZER DFREE '\n'					{mem_prof.Create_new_analyzer(unique_ptr<Double_Free_Analyzer> (new Double_Free_Analyzer()));}
		| ADD ANALYZER PRINT '\n'					{mem_prof.Create_new_analyzer(unique_ptr<Print_Analyzer> (new Print_Analyzer()));}
		| ADD FILTER SIZE NUMBER TEXT'\n'			{mem_prof.Create_new_size_filter_cli($4,$5);}
		| ADD ANALYZER NUMBER PATTERN NUMBER '\n'	{mem_prof.Add_analyzer_to_pattern($3,$5);}
		| ADD ANALYZER NUMBER PATTERN TEXT '\n'		{mem_prof.Add_analyzer_to_pattern_by_name($3,$5);}
		| REMOVE ANALYZER NUMBER '\n'				{mem_prof.Remove_analyzer($3);}
		| REMOVE ANALYZER NUMBER PATTERN NUMBER '\n'{mem_prof.Remove_analyzer_from_pattern($3,$5);}
		| REMOVE ANALYZER NUMBER PATTERN TEXT '\n'	{mem_prof.Remove_analyzer_from_pattern_by_name($3,$5);}
		| ADD FILTER NUMBER PATTERN NUMBER '\n'		{mem_prof.Add_filter_to_pattern($3,$5);}
		| ADD FILTER NUMBER PATTERN TEXT '\n'		{mem_prof.Add_filter_to_pattern_by_name($3,$5);}
		| REMOVE FILTER NUMBER '\n'					{mem_prof.Remove_filter($3);}
		| REMOVE FILTER NUMBER PATTERN NUMBER '\n'	{mem_prof.Remove_filter_from_pattern($3,$5);}
		| REMOVE FILTER NUMBER PATTERN TEXT '\n'	{mem_prof.Remove_filter_from_pattern_by_name($3,$5);}
		| PROCESS NUMBER ANALYZE PATTERN NUMBER '\n'{mem_prof.Run_pattern($5,$2);}
		| PROCESS NUMBER ANALYZE PATTERN TEXT '\n'	{mem_prof.Run_pattern($5,$2);}
		| PROCESS ALL ANALYZE PATTERN NUMBER '\n'	{mem_prof.Run_pattern_all_process($5);}
		| PROCESS ALL ANALYZE PATTERN TEXT '\n'		{mem_prof.Run_pattern_all_process($5);}
		| PRINT ANALYZER ALL '\n'					{mem_prof.Print_analyzers();}
		| PRINT PATTERN ALL '\n'					{mem_prof.Print_patterns();}
		| PRINT FILTER ALL '\n'						{mem_prof.Print_filters();}
		| HELP '\n'									{Print_help();}
		| EXIT_COMMAND '\n'    						{exit(1);}
		| '\n'										{;}
		| error '\n'								{cout << "Unrecognized command!" << endl;}
		;
	

%%



#define path_to_FIFO "/dev/mem_prof_fifo"

using namespace std;

static Memory_Profiler mem_prof(path_to_FIFO);
static pthread_t FIFO_read_thread_id;


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


	cout << endl << "Recognized commands:" << endl;

	cout <<"print process number" << endl;
	cout <<"print process all" << endl;
	cout <<"print process alive" << endl;
	cout <<"print process profiled" << endl;
	cout <<"print process number bt" << endl;
	cout <<"print process all bt" << endl;
	cout <<"print analyzer all" << endl;
	cout <<"print filter all" << endl;
	cout <<"print pattern all" << endl;
	
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
	
	cout <<"save process number symbols" << endl;
	cout <<"save process number map" << endl;
	cout <<"save process number bt" << endl;
	cout <<"save process all bt" << endl;
		
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
		usleep(200);
	}
	return 0;
}


int main() {

	signal(SIGINT, signal_callback_handler);

	int err = pthread_create(&FIFO_read_thread_id, NULL, &Read_FIFO_thread, NULL);
	if (err) {
		cout << "Thread creation failed error: " << err << endl;
	} else {
		cout << "Read_FIFO_thread created" << endl;
	}

	cout << ">> "; 
	return yyparse();
}
