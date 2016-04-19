dialect=-I/home/egezhus/binutils/build/include -DPACKAGE -DPACKAGE_VERSION


objects = Debug/Memory_Profiler_class.o Debug/Memory_Profiler_process.o Debug/Memory_Profiler_printing_functions.o \
          Debug/Memory_Profiler_memory_map.o Debug/Memory_Profiler_symbol_table.o \
          Debug/Memory_Profiler_analyzer.o \
          Debug/Memory_Profiler_pattern.o \
          Debug/Memory_Profiler_filter.o \
		  Debug/Memory_Profiler_tab.tab.o Debug/Memory_Profiler_lex.yy.o
		  
		  

          
Debug/Memory_Profiler_class.o : Memory_Profiler_class.cc
	g++ -c -g3 $(dialect) Memory_Profiler_class.cc -o Debug/Memory_Profiler_class.o
	
Debug/Memory_Profiler_process.o : Memory_Profiler_process.cc
	g++ -c -g3 $(dialect) Memory_Profiler_process.cc -o Debug/Memory_Profiler_process.o
		
Debug/Memory_Profiler_printing_functions.o : Memory_Profiler_printing_functions.cc
	g++ -c -g3 $(dialect) Memory_Profiler_printing_functions.cc -o Debug/Memory_Profiler_printing_functions.o

Debug/Memory_Profiler_memory_map.o : Memory_Profiler_memory_map.cc
	g++ -c -g3 $(dialect) Memory_Profiler_memory_map.cc -o Debug/Memory_Profiler_memory_map.o

Debug/Memory_Profiler_symbol_table.o : Memory_Profiler_symbol_table.cc
	g++ -c -g3 $(dialect) Memory_Profiler_symbol_table.cc -o Debug/Memory_Profiler_symbol_table.o

Debug/Memory_Profiler_analyzer.o: Memory_Profiler_analyzer.cc
	g++ -c -g3 $(dialect) Memory_Profiler_analyzer.cc -o Debug/Memory_Profiler_analyzer.o

Debug/Memory_Profiler_pattern.o: Memory_Profiler_pattern.cc
	g++ -c -g3 $(dialect) Memory_Profiler_pattern.cc -o Debug/Memory_Profiler_pattern.o

Debug/Memory_Profiler_filter.o: Memory_Profiler_filter.cc
	g++ -c -g3 $(dialect) Memory_Profiler_filter.cc -o Debug/Memory_Profiler_filter.o
	
Debug/Memory_Profiler_tab.tab.o: Memory_Profiler_language.y
	yacc -o Debug/Memory_Profiler_tab.tab.cc -d Memory_Profiler_language.y
	g++ -c -g3 $(dialect) Debug/Memory_Profiler_tab.tab.cc -o Debug/Memory_Profiler_tab.tab.o

Debug/Memory_Profiler_lex.yy.o: Memory_Profiler_lex.l 
	 lex -o Debug/Memory_Profiler_lex.yy.c Memory_Profiler_lex.l
	 g++ -c -g3 $(dialect) Debug/Memory_Profiler_lex.yy.c -o Debug/Memory_Profiler_lex.yy.o	

Debug/Memory_Profiler : $(objects)
	g++ $(dialect) -o Debug/Memory_Profiler -g3 $(objects) -lpthread -lrt -lbfd	-ldl
	
	
Debug/Memory_Profiler_STATIC: $(objects)
	g++ $(dialect) -o Debug/Memory_Profiler_STATIC -g3 $(objects) \
	-lrt \
	-lpthread \
	/home/egezhus/binutils/build/lib/libbfd.a \
	/home/egezhus/binutils/build/libiberty/libiberty.a \
	/home/egezhus/binutils/build/zlib/libz.a \
	-ldl 


all: Debug/Memory_Profiler Debug/Memory_Profiler_STATIC

clean:
	rm -rf Debug/Memory_*
