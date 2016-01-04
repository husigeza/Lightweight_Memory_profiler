# All Target
all: libMemory_Profiler_IMM_shared_library.so.1.0 libMemory_Profiler_shared_library.so.1.0

#sources
sources = ../Memory_Profiler_shared_library.c
#headers
headers = ../Memory_Profiler_shared_library.h
#object files
object = Memory_Profiler_shared_library.o
object_IMM = Memory_Profiler_IMM_shared_library.o

# Rules
Memory_Profiler_IMM_shared_library.o : $(sources) $(headers)
	@echo 'Building target: $@'
	gcc-4.4 -O0 -w -c -fPIC $(sources) -DSTART_PROF_IMM -o Memory_Profiler_IMM_shared_library.o
	@echo 'Finished building target: $@'
	@echo ' '
	
Memory_Profiler_shared_library.o : $(sources) $(headers)
	@echo 'Building target: $@'
	gcc-4.4 -O0 -w -c -fPIC $(sources)
	@echo 'Finished building target: $@' -o Memory_Profiler_shared_library.o
	@echo ' '
	
libMemory_Profiler_IMM_shared_library.so.1.0 : $(object_IMM)
	@echo 'Building target: $@'
	gcc-4.4 -O0 -Wall -rdynamic -shared -Wl,-soname=libMemory_Profiler_IMM_shared_library.so.1 -o libMemory_Profiler_IMM_shared_library.so.1.0 $(object_IMM) -lc -ldl -lrt -lpthread
	@echo 'Finished building target: $@'
	@echo ' '
	
libMemory_Profiler_shared_library.so.1.0 : $(object)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc-4.4 -O0 -Wall -rdynamic -shared -Wl,-soname=libMemory_Profiler_shared_library.so.1 -o libMemory_Profiler_shared_library.so.1.0 $(object) -lc -ldl -lrt -lpthread
	@echo 'Finished building target: $@'
	@echo ' '

	
# Other Targets
install: 
	@echo 'Coping to /usr/local/lib'
	cp libMemory_Profiler_shared_library.so.1.0 /usr/local/lib
	cp libMemory_Profiler_IMM_shared_library.so.1.0 /usr/local/lib
	
	ln -sf /usr/local/lib/libMemory_Profiler_shared_library.so.1.0 /usr/local/lib/libMemory_Profiler_shared_library.so.1
	ln -sf /usr/local/lib/libMemory_Profiler_IMM_shared_library.so.1.0 /usr/local/lib/libMemory_Profiler_IMM_shared_library.so.1
			
	ln -sf /usr/local/lib/libMemory_Profiler_shared_library.so.1.0 /usr/local/lib/libMemory_Profiler_shared_library.so
	ln -sf /usr/local/lib/libMemory_Profiler_IMM_shared_library.so.1.0 /usr/local/lib/libMemory_Profiler_IMM_shared_library.so
	
	ldconfig

clean:
	 rm -f *.o *.so.* *.d

	 

.PHONY: all clean install
.SECONDARY:

