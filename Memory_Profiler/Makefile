CUSTOM_BFD = /home/egezhus/binutils/build

Memory_Profiler_tab.tab.cc: Memory_Profiler_language.y
	yacc -o Memory_Profiler_tab.tab.cc -d Memory_Profiler_language.y

Memory_Profiler_lex.yy.c: Memory_Profiler_lex.l 
	 lex -o Memory_Profiler_lex.yy.c Memory_Profiler_lex.l
	
Memory_Profiler: Memory_Profiler_tab.tab.cc Memory_Profiler_lex.yy.c
	g++ -o Memory_Profiler -g3 *.cc *.c -lpthread -lrt -lbfd -ldl
		
Memory_Profiler_STATIC: Memory_Profiler_tab.tab.cc Memory_Profiler_lex.yy.c
	g++ -o Memory_Profiler_STATIC -g3 -DPACKAGE -DPACKAGE_VERSION -I$(CUSTOM_BFD)/include *.cc *.c \
	-lrt \
	-lpthread \
	$(CUSTOM_BFD)/lib/libbfd.a \
	$(CUSTOM_BFD)/libiberty/libiberty.a \
	$(CUSTOM_BFD)/zlib/libz.a \
	-ldl 

all:  Memory_Profiler Memory_Profiler_STATIC

clean:
	rm -rf Memory_Profiler Memory_Profiler_STATIC
