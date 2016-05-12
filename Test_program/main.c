#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdbool.h>


int main()
{
int i = 0;
int *ptr1;

	//set_user_profiling_flag(true);
	//ptr1 = malloc(100);
	//set_user_profiling_flag(false);

	//while(i < 3){
    	ptr1 = malloc(sizeof(int));
    	//if(i%2){
    		free(ptr1);
    	//}
    	//i++;
   //}

    sleep(1);
	exit(0);

}







