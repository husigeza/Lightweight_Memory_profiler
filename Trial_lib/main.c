#include <stdio.h>
#include <stdlib.h>

int i = 0;

void __attribute__ ((constructor)) init324() {

	printf("TRIAL lib init!\n");
	int *ptr = (int*)malloc(sizeof(int));

}

int func5(){
	int *ptr = (int*)malloc(i*sizeof(int));
	//free (ptr);
	printf("i: %d\n",i);
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
