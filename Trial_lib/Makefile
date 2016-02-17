#sources
sources = main.c
#object files
object = Debug/main.o

# Rules
Debug/main.o : $(sources) #$(headers)
	@echo ' '
	gcc -O0 -w -c -fPIC $(sources) -o Debug/main.o
	@echo ' '
		
Debug/libTrial_lib.so.1.0 : $(object)
	gcc -O0 -Wall -rdynamic -shared -Wl,-soname=libTrial_lib.so.1 -o Debug/libTrial_lib.so.1.0 $(object) -lc
	@echo ' '
		
# Other Targets
install: 
	@echo 'Coping to /usr/local/lib'
	cp Debug/libTrial_lib.so.1.0 /usr/local/lib
	
	ln -sf /usr/local/lib/libTrial_lib.so.1.0 /usr/local/lib/libTrial_lib.so.1
	ln -sf /usr/local/lib/libTrial_lib.so.1.0 /usr/local/lib/libTrial_lib.so
			
	ldconfig

# All Target
all: Debug/libTrial_lib.so.1.0

clean:
	 rm -rf Debug/*.o Debug/*.so.*

	 
.PHONY: all clean install
.SECONDARY:

