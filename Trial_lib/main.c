#include <stdio.h>
#include <stdlib.h>

int i = 0;
static int *pointer;

void __attribute__ ((constructor)) init324() {

	printf("TRIAL lib init!\n");
	//int *ptr = (int*)malloc(sizeof(int));

}

int func5(){
	pointer = (int *)realloc(pointer,sizeof(int));
	//free (ptr);

	i++;

	return i;
}

int func4(){

	return func5();
}

int func3(){

	return func4();
}

int func2(){

	return func3();
}

int func1(){

	//printf("func1\n");
	return func2();
}
