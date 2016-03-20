#include <stdio.h>
#include <stdlib.h>

int i = 0;
static int *pointer;
static int *ptr;

void __attribute__ ((constructor)) init324() {

	printf("TRIAL lib init!\n");
	ptr = (int*)malloc(sizeof(int));

}

int func5(){
	pointer = (int *)calloc(15,sizeof(int));


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
