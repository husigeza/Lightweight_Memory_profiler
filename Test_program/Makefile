# sources
source = ../main.c
# Memory profiler shared library
memory_profiler_shared_lib = Memory_profiler_shared_library
memory_profiler_shared_lib_IMM = Memory_profiler_IMM_shared_library

# Tool invocations
Test_program: $(source)
	#@echo 'Building target: Test_program'
	#@echo 'Invoking: GCC C Linker'
	gcc $(source) -Wl,-Bdynamic,--no-as-needed -l$(memory_profiler_shared_lib) -o Test_Program -lpthread -ldl
	#@echo 'Finished building target: Test_program'
	@echo ' '
Test_program_IMM: $(source)
	#@echo 'Building target: Test_program_IMM'
	#@echo 'Invoking: GCC C Linker'
	gcc $(source) -Wl,-Bdynamic,--no-as-needed -l$(memory_profiler_shared_lib_IMM) -o Test_Program_IMM -lpthread -ldl
	#@echo 'Finished building target: Test_program_IMM'
	#@echo ' '

.PHONY: all clean
# All Target
all: Test_program Test_program_IMM
# Other Targets
clean: 
	rm Test_Program Test_Program_IMM
	@echo ' '


