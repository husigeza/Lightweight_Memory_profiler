
#sources
sources = Memory_Profiler_shared_library.c
#headers
headers = Memory_Profiler_shared_library.h

#object files
object = Debug/Memory_Profiler_shared_library.o
object_IMM = Debug/Memory_Profiler_IMM_shared_library.o

# Rules
Debug/Memory_Profiler_IMM_shared_library.o : $(sources) $(headers)
	@echo ' '	
	gcc -O0 -w -c -fPIC $(sources) -DSTART_PROF_IMM -o Debug/Memory_Profiler_IMM_shared_library.o	
	@echo ' '
	
Debug/Memory_Profiler_shared_library.o : $(sources) $(headers)
	gcc -O0 -w -c -fPIC $(sources) -o Debug/Memory_Profiler_shared_library.o
	@echo ' '
	
Debug/libMemory_Profiler_IMM_shared_library.so.1.0 : $(object_IMM)
	gcc -O0 -Wall -rdynamic -shared -Wl,-soname=libMemory_Profiler_IMM_shared_library.so.1 -o Debug/libMemory_Profiler_IMM_shared_library.so.1.0 $(object_IMM) -lc -ldl -lrt -lpthread
	@echo ' '
	
Debug/libMemory_Profiler_shared_library.so.1.0 : $(object)
	gcc -O0 -Wall -rdynamic -shared -Wl,-soname=libMemory_Profiler_shared_library.so.1 -o Debug/libMemory_Profiler_shared_library.so.1.0 $(object) -lc -ldl -lrt -lpthread
	@echo ' '
	
# Other Targets
install: 
	@echo 'Coping to /usr/local/lib'
	cp Debug/libMemory_Profiler_shared_library.so.1.0 /usr/local/lib
	cp Debug/libMemory_Profiler_IMM_shared_library.so.1.0 /usr/local/lib
	
	ln -sf /usr/local/lib/libMemory_Profiler_shared_library.so.1.0 /usr/local/lib/libMemory_Profiler_shared_library.so.1
	ln -sf /usr/local/lib/libMemory_Profiler_IMM_shared_library.so.1.0 /usr/local/lib/libMemory_Profiler_IMM_shared_library.so.1
			
	ln -sf /usr/local/lib/libMemory_Profiler_shared_library.so.1.0 /usr/local/lib/libMemory_Profiler_shared_library.so
	ln -sf /usr/local/lib/libMemory_Profiler_IMM_shared_library.so.1.0 /usr/local/lib/libMemory_Profiler_IMM_shared_library.so
	
	ldconfig

# All Target
all: Debug/libMemory_Profiler_IMM_shared_library.so.1.0 Debug/libMemory_Profiler_shared_library.so.1.0

clean:
	 rm -f Debug/*.o Debug/*.so.*
