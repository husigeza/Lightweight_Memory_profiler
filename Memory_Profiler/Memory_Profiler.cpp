#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "Memory_Profiler.h"


#define fifo_path "/home/egezhus/mem_prof_fifo"


using namespace std;

static Memory_Profiler mem_prof;

Memory_Profiler::Memory_Profiler(){

    if(mkfifo(fifo_path, 0666) == -1){

        if(errno == EEXIST) {
            cout<< "FIFO already exists" << endl;
        }
        else {
            cout<< "Failed creating FIFO" << "errno: " << errno << endl;
        }
        return;
    }
    cout<< "FIFO is created" << endl;
}


Memory_Profiler::~Memory_Profiler() {

   /* for(int i =0; i<this->NumOfProcesses;i++){
            delete(this->ProcessesList[i]);
    }
    delete(this->ProcessesList);*/

    unlink(fifo_path);

}

void Memory_Profiler::Print_all_processes()const{

        for(int i = 0; i< All_processes.size(); i++){

            cout << All_processes[i] << endl;
        }
    cout<< "Number of processes: " << All_processes.size() << endl;
}

void Memory_Profiler::Print_profiled_processes()const{

        for(int i = 0; i< Profiled_processes.size(); i++){

            cout << Profiled_processes[i] << endl;
        }
    cout<< "Number of processes: " << Profiled_processes.size() << endl;
}


void Memory_Profiler::Add_Process_to_list(uint32_t PID) {

    if(std::find(All_processes.begin(), All_processes.end(), PID) == All_processes.end()) {
        All_processes.push_back(PID);
    }
    else {

        //cout<< "Process is already added to process list: " << PID << endl;
    }

}
void Memory_Profiler::Add_process_to_profiling(uint32_t PID) {

    if(std::find(Profiled_processes.begin(), Profiled_processes.end(), PID) == Profiled_processes.end()) {
        Profiled_processes.push_back(PID);
    }
    else {

        cout<< "Process is already profiled" << endl;

    }
}

void Memory_Profiler::Add_all_process_to_profiling(){


    for(int i = 0; i < All_processes.size(); i++){

        Add_process_to_profiling(All_processes[i]);
    }
}

std::vector<uint32_t> Memory_Profiler::Get_profiled_processes_list()const{

    return Profiled_processes;
}

std::vector<uint32_t> Memory_Profiler::Get_all_processes_list()const{

    return All_processes;
}


void Memory_Profiler::Read_FIFO(){

    int mem_prof_fifo = open(fifo_path, O_RDONLY);
    char buffer[5];
    size_t buff_size = 5;
    uint32_t pid;
    int res;

        if(mem_prof_fifo != -1) {
            while((res = read(mem_prof_fifo,&buffer, sizeof(buffer))) != 0){

                if(res != -1){
                    pid = stoi(buffer,&buff_size);
                    Add_Process_to_list(pid);
                    //cout << "Reading the FIFO was succesfull" << endl;
                }
                else {
                    printf("Failed reading the FIFO\n");
                    break;
                }
            }
            //cout << "FIFO is empty" << endl;
        }
        else {

        cout << "Failed opening the FIFO, errno: " << errno << endl;

        }
        close(mem_prof_fifo);
}

Process_handler::Process_handler(uint32_t PID){

        this->PID = PID;
        Init_shared_memory();
}

Process_handler::~Process_handler(){

    munmap(malloc_struct,sizeof(malloc_struct_t));

}

void Process_handler::Init_shared_memory(){

    char PID_string[5];
    sprintf(PID_string,"%d",this->PID);

    int shared_memory = shm_open(PID_string, O_RDWR , NULL);
    if(shared_memory < 0){

        printf("Error while creating shared memory:%d \n",errno);

    }

    int err = ftruncate(shared_memory,sizeof(malloc_struct_t));
    if(err < 0){

        printf("Error while truncating shared memory: %d \n",errno);

    }

    malloc_struct = (malloc_struct_t*)mmap(NULL,sizeof(malloc_struct_t),PROT_READ,MAP_SHARED,shared_memory,0);
    if(malloc_struct == MAP_FAILED) {

        printf("Failed mapping the shared memory: %d \n",errno);

    }

}

void Process_handler::Print_shared_memory(){

    printf("malloc_struct.len = %d\n",malloc_struct->len);

}

/*void* Read_FIFO_thread(void *arg) {

    while(true){
    mem_prof.Read_FIFO();

    sleep(2);
    }

}*/

int main()
{

    /*int err = pthread_create(&hearthbeat_thread_id, NULL, &Read_FIFO_thread, NULL);
        if(err){
            printf("Thread creation failed error:%d \n",err);
        }
        else {
            printf("Thread created\n");
        }
*/


        mem_prof.Read_FIFO();
        mem_prof.Print_all_processes();
        mem_prof.Add_all_process_to_profiling();


    while(1){
        //sleep(3);
    }

    return 0;
}
