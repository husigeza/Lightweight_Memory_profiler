# sources
source = main.c

# Memory profiler shared library
Memory_Profiler_shared_lib = Memory_Profiler_shared_library
Memory_Profiler_shared_lib_IMM = Memory_Profiler_IMM_shared_library
Memory_Profiler_USERPROF_shared_lib = Memory_Profiler_USERPROF_shared_library
Memory_Profiler_USERPROF_shared_lib_IMM = Memory_Profiler_USERPROF_IMM_shared_library

# Tool invocations
Debug/Test_program: $(source)	
	gcc $(source) -w -o Debug/Test_Program -ldl
	
Debug/Test_program_IMM: $(source)
	gcc -g3 $(source) -w -l$(Memory_Profiler_shared_lib_IMM) -o Debug/Test_Program_IMM -ldl
	
Debug/Test_Program_USER_IMM: $(source)
	gcc -g3 $(source) -w -l$(Memory_Profiler_USERPROF_shared_lib_IMM) -o Debug/Test_Program_USER_IMM -ldl
		
# All Target
all: Debug/Test_program_IMM Debug/Test_Program_USER_IMM

# Other Targets
clean: 
	rm Debug/Test_Program Debug/Test_Program_IMM Debug/Test_Program_USER_IMM
	@echo ' '

	
	