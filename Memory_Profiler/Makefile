gcc = /home/egezhus/GCC_4_3_bin/bin/gcc
g++ = /home/egezhus/GCC_4_3_bin/bin/g++	

objects = Debug/Memory_Profiler.o Debug/Memory_Profiler_class.o Debug/Memory_Profiler_process.o Debug/Memory_Profiler_printing_functions.o Debug/Memory_Profiler_memory_map.o Debug/Memory_Profiler_symbol_table.o

Debug/Memory_Profiler.o : Memory_Profiler.cc
	g++ -c Memory_Profiler.cc -std=c++0x -o Debug/Memory_Profiler.o 	

Debug/Memory_Profiler_class.o : Memory_Profiler_class.cc
	g++ -c Memory_Profiler_class.cc -std=c++0x -o Debug/Memory_Profiler_class.o

Debug/Memory_Profiler_process.o : Memory_Profiler_process.cc
	g++ -c Memory_Profiler_process.cc -std=c++0x -o Debug/Memory_Profiler_process.o
	
Debug/Memory_Profiler_printing_functions.o : Memory_Profiler_printing_functions.cc
	g++ -c Memory_Profiler_printing_functions.cc -std=c++0x -o Debug/Memory_Profiler_printing_functions.o
	
Debug/Memory_Profiler_memory_map.o : Memory_Profiler_memory_map.cc
	g++ -c Memory_Profiler_memory_map.cc -std=c++0x -o Debug/Memory_Profiler_memory_map.o
	
Debug/Memory_Profiler_symbol_table.o : Memory_Profiler_symbol_table.cc
	g++ -c Memory_Profiler_symbol_table.cc -std=c++0x -o Debug/Memory_Profiler_symbol_table.o

Debug/Memory_Profiler : $(objects)
	@echo ' '
	g++ $(objects) -std=c++0x -o Debug/Memory_Profiler -lrt -lpthread -lbfd

all: Debug/Memory_Profiler

clean:
	rm Debug/*.o
