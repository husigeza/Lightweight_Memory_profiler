# sources
source = main.c

# Memory profiler shared library
Memory_Profiler_shared_lib = Memory_Profiler_shared_library
Memory_Profiler_shared_lib_IMM = Memory_Profiler_IMM_shared_library
Memory_Profiler_USERPROF_shared_lib = Memory_Profiler_USERPROF_shared_library
Memory_Profiler_USERPROF_shared_lib_IMM = Memory_Profiler_USERPROF_IMM_shared_library

# Tool invocations
Debug/Test_program: $(source)
	@echo ' '
	gcc $(source) -w -Wl,-Bdynamic,--no-as-needed -l$(Memory_Profiler_shared_lib) -o Debug/Test_Program -lpthread -ldl -lTrial_lib
	@echo ' '
Debug/Test_program_IMM: $(source)
	gcc $(source) -w -Wl,-Bdynamic,--no-as-needed -l$(Memory_Profiler_USERPROF_shared_lib_IMM) -o Debug/Test_Program_IMM -lpthread -ldl -lTrial_lib

# All Target
all: Debug/Test_program Debug/Test_program_IMM

# Other Targets
clean: 
	rm Debug/Test_Program Debug/Test_Program_IMM
	@echo ' '
