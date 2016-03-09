dialect=


objects = Debug/Memory_Profiler_class.o Debug/Memory_Profiler_process.o Debug/Memory_Profiler_printing_functions.o \
          Debug/Memory_Profiler_memory_map.o Debug/Memory_Profiler_symbol_table.o \
          Debug/Memory_Profiler_analyzer.o \
          Debug/Memory_Profiler_pattern.o \
          Debug/Memory_Profiler_filter.o \
		  Debug/Memory_Profiler_tab.tab.o Debug/Memory_Profiler_lex.yy.o
		  
		  

          
Debug/Memory_Profiler_class.o : Memory_Profiler_class.cc
	g++ -c -g3 Memory_Profiler_class.cc $(dialect) -o Debug/Memory_Profiler_class.o
	
Debug/Memory_Profiler_process.o : Memory_Profiler_process.cc
	g++ -c -g3 Memory_Profiler_process.cc $(dialect) -o Debug/Memory_Profiler_process.o
		
Debug/Memory_Profiler_printing_functions.o : Memory_Profiler_printing_functions.cc
	g++ -c -g3 Memory_Profiler_printing_functions.cc $(dialect) -o Debug/Memory_Profiler_printing_functions.o

Debug/Memory_Profiler_memory_map.o : Memory_Profiler_memory_map.cc
	g++ -c -g3 Memory_Profiler_memory_map.cc $(dialect) -o Debug/Memory_Profiler_memory_map.o

Debug/Memory_Profiler_symbol_table.o : Memory_Profiler_symbol_table.cc
	g++ -c -g3 Memory_Profiler_symbol_table.cc $(dialect) -o Debug/Memory_Profiler_symbol_table.o

Debug/Memory_Profiler_analyzer.o: Memory_Profiler_analyzer.cc
	g++ -c -g3 Memory_Profiler_analyzer.cc $(dialect) -o Debug/Memory_Profiler_analyzer.o

Debug/Memory_Profiler_pattern.o: Memory_Profiler_pattern.cc
	g++ -c -g3 Memory_Profiler_pattern.cc $(dialect) -o Debug/Memory_Profiler_pattern.o

Debug/Memory_Profiler_filter.o: Memory_Profiler_filter.cc
	g++ -c -g3 Memory_Profiler_filter.cc $(dialect) -o Debug/Memory_Profiler_filter.o
	
Debug/Memory_Profiler_tab.tab.o: Memory_Profiler_language.y
	yacc -o Debug/Memory_Profiler_tab.tab.cc -d Memory_Profiler_language.y
	g++ -c -g3 Debug/Memory_Profiler_tab.tab.cc $(dialect) -o Debug/Memory_Profiler_tab.tab.o

Debug/Memory_Profiler_lex.yy.o: Memory_Profiler_lex.l 
	 lex -o Debug/Memory_Profiler_lex.yy.c Memory_Profiler_lex.l
	 g++ -c -g3 Debug/Memory_Profiler_lex.yy.c $(dialect) -o Debug/Memory_Profiler_lex.yy.o	

Debug/Memory_Profiler : $(objects)
	@echo ' '
	g++ -o Debug/Memory_Profiler -g3 $(dialect) $(objects) -lpthread -lrt -ldl -lbfd	

all: Debug/Memory_Profiler 

clean:
	rm -rf Debug/Memory_*
