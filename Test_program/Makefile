# sources
source = main.c
source_parser = testlex.l testyacc.y

# Memory profiler shared library
Memory_Profiler_shared_lib = Memory_Profiler_shared_library
Memory_Profiler_shared_lib_IMM = Memory_Profiler_IMM_shared_library
Memory_Profiler_USERPROF_shared_lib = Memory_Profiler_USERPROF_shared_library
Memory_Profiler_USERPROF_shared_lib_IMM = Memory_Profiler_USERPROF_IMM_shared_library

# Tool invocations
Debug/Test_program: $(source)
	@echo ' '
	#gcc $(source) -w -Wl,-Bdynamic,--no-as-needed -l$(Memory_Profiler_shared_lib) -o Debug/Test_Program -lpthread -ldl -lTrial_lib
	gcc $(source) -w -o Debug/Test_Program -lpthread -ldl -lTrial_lib
	@echo ' '
Debug/Test_program_IMM: $(source)
	gcc $(source) -w -Wl,-Bdynamic,--no-as-needed -l$(Memory_Profiler_shared_lib_IMM) -o Debug/Test_Program_IMM -lpthread -ldl -lTrial_lib
		
Debug/testtab.o: testyacc.y
	yacc -o test.tab.c -d testyacc.y
	gcc -c -g3 test.tab.c -o Debug/testtab.o

Debug/testlex.yy.o: testlex.l 
	 lex -o Debug/testprogram_lex.yy.c testlex.l
	 gcc -c -g3 Debug/testprogram_lex.yy.c $(dialect) -o Debug/testlex.yy.o		

Debug/Test_program_IMM_parser: Debug/testtab.o Debug/testlex.yy.o 
	gcc  Debug/testtab.o Debug/testlex.yy.o  -w -Wl,-Bdynamic,--no-as-needed -l$(Memory_Profiler_shared_lib_IMM) -o Debug/Test_Program_IMM_parser

# All Target
all: Debug/Test_program Debug/Test_program_IMM

parser: Debug/Test_program_IMM_parser

# Other Targets
clean: 
	rm Debug/Test_Program Debug/Test_Program_IMM Debug/Test_Program_IMM_parser
	@echo ' '
