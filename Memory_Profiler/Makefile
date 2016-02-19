root_path_ppc =		 /media/egezhus/Disk/crosscompilers/powerpc-linux-gnu
root_path_ppc_2 =	 /media/egezhus/Disk/crosscompilers/powerpc-e500v2-linux

gcc_ppc = $(root_path_ppc)/bin/powerpc-linux-gnu-gcc
g++_ppc = $(root_path_ppc)/bin/powerpc-linux-gnu-g++

gcc_ppc_2 = $(root_path_ppc_2)/bin/powerpc-e500v2-linux-gcc
g++_ppc_2 = $(root_path_ppc_2)/bin/powerpc-e500v2-linux-g++



bfd_include_ppc =    $(root_path_ppc)/binutils/include
bfd_config_ppc  =	 $(root_path_ppc)/binutils/bfd
libbfd_path_ppc =    $(root_path_ppc)/binutils/bfd
libiberty_path_ppc = $(root_path_ppc)/binutils/libiberty
libz_path_ppc =	     $(root_path_ppc)/binutils/zlib
lib_path_ppc =       $(root_path_ppc)/powerpc-linux-gnu/lib


bfd_include_ppc_2 =    	$(root_path_ppc_2)/binutils/include
bfd_config_ppc_2  =	 	$(root_path_ppc_2)/binutils/bfd
libbfd_path_ppc_2 =    	$(root_path_ppc_2)/binutils/bfd
libiberty_path_ppc_2 = 	$(root_path_ppc_2)/binutils/libiberty
libz_path_ppc_2 =	    $(root_path_ppc_2)/binutils/zlib
lib_path_ppc_2 =       	$(root_path_ppc_2)/powerpc-linux-gnu/lib


	
objects = Debug/Memory_Profiler_class.o Debug/Memory_Profiler_process.o Debug/Memory_Profiler_printing_functions.o \
          Debug/Memory_Profiler_memory_map.o Debug/Memory_Profiler_symbol_table.o \
          Debug/Memory_Profiler_analyzer.o \
          Debug/Memory_Profiler_analyzer_pattern.o \
          Debug/Memory_Profiler_filter.o \
		  Debug/Memory_Profiler_tab.tab.o Debug/Memory_Profiler_lex.yy.o
		  
objects_ppc = Debug/Memory_Profiler_class_ppc.o Debug/Memory_Profiler_process_ppc.o Debug/Memory_Profiler_printing_functions_ppc.o \
          Debug/Memory_Profiler_memory_map_ppc.o Debug/Memory_Profiler_symbol_table_ppc.o \
          Debug/Memory_Profiler_analyzer_ppc.o \
          Debug/Memory_Profiler_analyzer_pattern_ppc.o \
          Debug/Memory_Profiler_filter_ppc.o \
		  Debug/Memory_Profiler_tab_ppc.tab.o Debug/Memory_Profiler_lex_ppc.yy.o

objects_ppc_2 = Debug/Memory_Profiler_class_ppc_2.o Debug/Memory_Profiler_process_ppc_2.o Debug/Memory_Profiler_printing_functions_ppc_2.o \
          Debug/Memory_Profiler_memory_map_ppc_2.o Debug/Memory_Profiler_symbol_table_ppc_2.o \
          Debug/Memory_Profiler_analyzer_ppc_2.o \
          Debug/Memory_Profiler_analyzer_pattern_ppc_2.o \
          Debug/Memory_Profiler_filter_ppc_2.o \
		  Debug/Memory_Profiler_tab_ppc_2.tab.o Debug/Memory_Profiler_lex_ppc_2.yy.o
          
Debug/Memory_Profiler_class.o : Memory_Profiler_class.cc
	g++ -c -g3 Memory_Profiler_class.cc -std=c++11 -o Debug/Memory_Profiler_class.o
	
Debug/Memory_Profiler_class_ppc.o : Memory_Profiler_class.cc
	$(g++_ppc) -c -g3 Memory_Profiler_class.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_class_ppc.o
	
Debug/Memory_Profiler_class_ppc_2.o : Memory_Profiler_class.cc
	$(g++_ppc_2) -c -g3 Memory_Profiler_class.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_class_ppc_2.o

Debug/Memory_Profiler_process.o : Memory_Profiler_process.cc
	g++ -c -g3 Memory_Profiler_process.cc -std=c++11 -o Debug/Memory_Profiler_process.o
	
Debug/Memory_Profiler_process_ppc.o : Memory_Profiler_process.cc
	$(g++_ppc) -c -g3 Memory_Profiler_process.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_process_ppc.o
	
Debug/Memory_Profiler_process_ppc_2.o : Memory_Profiler_process.cc
	$(g++_ppc_2) -c -g3 Memory_Profiler_process.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_process_ppc_2.o
	
Debug/Memory_Profiler_printing_functions.o : Memory_Profiler_printing_functions.cc
	g++ -c -g3 Memory_Profiler_printing_functions.cc -std=c++11 -o Debug/Memory_Profiler_printing_functions.o

Debug/Memory_Profiler_printing_functions_ppc.o : Memory_Profiler_printing_functions.cc
	$(g++_ppc) -c -g3 Memory_Profiler_printing_functions.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_printing_functions_ppc.o

Debug/Memory_Profiler_printing_functions_ppc_2.o : Memory_Profiler_printing_functions.cc
	$(g++_ppc_2) -c -g3 Memory_Profiler_printing_functions.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_printing_functions_ppc_2.o
	
Debug/Memory_Profiler_memory_map.o : Memory_Profiler_memory_map.cc
	g++ -c -g3 Memory_Profiler_memory_map.cc -std=c++11 -o Debug/Memory_Profiler_memory_map.o

Debug/Memory_Profiler_memory_map_ppc.o : Memory_Profiler_memory_map.cc
	$(g++_ppc) -c -g3 Memory_Profiler_memory_map.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_memory_map_ppc.o

Debug/Memory_Profiler_memory_map_ppc_2.o : Memory_Profiler_memory_map.cc
	$(g++_ppc_2) -c -g3 Memory_Profiler_memory_map.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_memory_map_ppc_2.o
	
Debug/Memory_Profiler_symbol_table.o : Memory_Profiler_symbol_table.cc
	g++ -c -g3 Memory_Profiler_symbol_table.cc -std=c++11 -o Debug/Memory_Profiler_symbol_table.o

Debug/Memory_Profiler_symbol_table_ppc.o : Memory_Profiler_symbol_table.cc
	$(g++_ppc) -c -g3 Memory_Profiler_symbol_table.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_symbol_table_ppc.o

Debug/Memory_Profiler_symbol_table_ppc_2.o : Memory_Profiler_symbol_table.cc
	$(g++_ppc_2) -c -g3 Memory_Profiler_symbol_table.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_symbol_table_ppc_2.o
	
Debug/Memory_Profiler_analyzer.o: Memory_Profiler_analyzer.cc
	g++ -c -g3 Memory_Profiler_analyzer.cc -std=c++11 -o Debug/Memory_Profiler_analyzer.o

Debug/Memory_Profiler_analyzer_ppc.o: Memory_Profiler_analyzer.cc
	$(g++_ppc) -c -g3 Memory_Profiler_analyzer.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_analyzer_ppc.o

Debug/Memory_Profiler_analyzer_ppc_2.o:	Memory_Profiler_analyzer.cc
	$(g++_ppc_2) -c -g3 Memory_Profiler_analyzer.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_analyzer_ppc_2.o

Debug/Memory_Profiler_analyzer_pattern.o: Memory_Profiler_analyzer_pattern.cc
	g++ -c -g3 Memory_Profiler_analyzer_pattern.cc -std=c++11 -o Debug/Memory_Profiler_analyzer_pattern.o

Debug/Memory_Profiler_analyzer_pattern_ppc.o: Memory_Profiler_analyzer_pattern.cc
	$(g++_ppc) -c -g3 Memory_Profiler_analyzer_pattern.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_analyzer_pattern_ppc.o

Debug/Memory_Profiler_analyzer_pattern_ppc_2.o:	Memory_Profiler_analyzer_pattern.cc
	$(g++_ppc_2) -c -g3 Memory_Profiler_analyzer_pattern.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_analyzer_pattern_ppc_2.o

Debug/Memory_Profiler_filter.o: Memory_Profiler_filter.cc
	g++ -c -g3 Memory_Profiler_filter.cc -std=c++11 -o Debug/Memory_Profiler_filter.o

Debug/Memory_Profiler_filter_ppc.o: Memory_Profiler_filter.cc
	$(g++_ppc) -c -g3 Memory_Profiler_filter.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_filter_ppc.o

Debug/Memory_Profiler_filter_ppc_2.o: Memory_Profiler_filter.cc
	$(g++_ppc_2) -c -g3 Memory_Profiler_filter.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_filter_ppc_2.o
		
		
Debug/Memory_Profiler_tab.tab.o: Memory_Profiler_language.y
	yacc -o Debug/Memory_Profiler_tab.tab.cc -d Memory_Profiler_language.y
	g++ -c -g3 Debug/Memory_Profiler_tab.tab.cc -std=c++11 -o Debug/Memory_Profiler_tab.tab.o

Debug/Memory_Profiler_tab_ppc.tab.o: Memory_Profiler_language.y
	yacc -o Debug/Memory_Profiler_tab.tab.cc -d Memory_Profiler_language.y
	$(g++_ppc) -c -g3 Debug/Memory_Profiler_tab.tab.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_tab_ppc.tab.o

Debug/Memory_Profiler_tab_ppc_2.tab.o: Memory_Profiler_language.y
	yacc -o Debug/Memory_Profiler_tab.tab.cc -d Memory_Profiler_language.y
	$(g++_ppc_2) -c -g3 Debug/Memory_Profiler_tab.tab.cc -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_tab_ppc_2.tab.o

Debug/Memory_Profiler_lex.yy.o: Memory_Profiler_lex.l 
	 lex -o Debug/Memory_Profiler_lex.yy.c Memory_Profiler_lex.l
	 g++ -c -g3 Debug/Memory_Profiler_lex.yy.c -std=c++11 -o Debug/Memory_Profiler_lex.yy.o	

Debug/Memory_Profiler_lex_ppc.yy.o: Memory_Profiler_lex.l 
	 lex -o Debug/Memory_Profiler_lex.yy.c Memory_Profiler_lex.l
	 $(g++_ppc) -c -g3 Debug/Memory_Profiler_lex.yy.c -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc) -I$(bfd_config_ppc) -o Debug/Memory_Profiler_lex_ppc.yy.o	

Debug/Memory_Profiler_lex_ppc_2.yy.o: Memory_Profiler_lex.l 
	 lex -o Debug/Memory_Profiler_lex.yy.c Memory_Profiler_lex.l
	 $(g++_ppc_2) -c -g3 Debug/Memory_Profiler_lex.yy.c -std=c++11 -Dbfd_need_config_h -I$(bfd_include_ppc_2) -I$(bfd_config_ppc_2) -o Debug/Memory_Profiler_lex_ppc_2.yy.o	

Debug/Memory_Profiler : $(objects)
	@echo ' '
	g++ -o Debug/Memory_Profiler -g3 -std=c++11 $(objects) -lpthread -lrt -ldl -lbfd	

Debug/Memory_Profiler_ppc : $(objects_ppc)
	@echo ' '
	$(g++_ppc) -o Debug/Memory_Profiler_ppc -g3 -std=c++11 $(objects_ppc) -static-libgcc -static \
	$(libbfd_path_ppc)/libbfd.a \
	$(libiberty_path_ppc)/libiberty.a \
	$(libz_path_ppc)/libz.a \
	-Wl,-Bstatic,-L,$(lib_path_ppc) \
	-ldl \
	-lrt \
	-lpthread 
 
Debug/Memory_Profiler_ppc_2 : $(objects_ppc_2)
	@echo ' '
	$(g++_ppc_2) -o Debug/Memory_Profiler_ppc_2 -g3 -std=c++11 $(objects_ppc_2) -static-libgcc -static \
	$(libbfd_path_ppc_2)/libbfd.a \
	$(libiberty_path_ppc_2)/libiberty.a \
	$(libz_path_ppc_2)/libz.a \
	-Wl,-Bstatic,-L,$(lib_path_ppc_2) \
	-ldl \
	-lrt \
	-lpthread 	

all: Debug/Memory_Profiler_ppc Debug/Memory_Profiler_ppc_2 Debug/Memory_Profiler 

clean:
	rm -rf Debug/Memory_*
