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
#include <map>
#include <signal.h>
#include <pthread.h>

#include "Memory_Profiler.h"


#define fifo_path "/home/egezhus/mem_prof_fifo"


using namespace std;

static Memory_Profiler mem_prof;
static pthread_t FIFO_read_thread_id;

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

    unlink(fifo_path);

}

void Memory_Profiler::Print_all_processes()const{


    cout<< "Printing all processes" << endl;
        for(auto& element: Processes){

            cout << "PID: " << element.first << endl;
        }
    cout<< "Number of processes: " << Processes.size() << endl;
}

void Memory_Profiler::Print_profiled_processes(){

    cout<< "Printing profiled" << endl;
        for(auto& element: Processes){

            if(element.second.Get_profiled() == true) {
                cout << element.first << endl;
        }
    }
}


void Memory_Profiler::Add_Process_to_list(pid_t PID) {

    if(Processes.find(PID) == Processes.end()){

    	Process_handler process(PID);
    	Processes.insert(pair<pid_t,Process_handler>(PID,process));
        /*map<pid_t,Process_handler>::iterator it = Processes.find(PID);
        it->second.Init_shared_memory();*/
    }
    else {

        //cout<< "Process is already added to process list: " << PID << endl;
    }

}
void Memory_Profiler::Add_process_to_profiling(pid_t PID) {

    map<pid_t,Process_handler>::iterator it = Processes.find(PID);

    if(it->second.Get_profiled() == false) {
        Processes.find(PID)->second.Set_profiled(true);
    }
    else {

        //cout<< "Process is already profiled: " << PID <<endl;

    }
}

void Memory_Profiler::Add_all_process_to_profiling(){

    map<pid_t,Process_handler>::iterator it;

    for(it = Processes.begin(); it != Processes.end(); it++){

        Add_process_to_profiling(it->first);
    }
}

/*std::vector<uint32_t> Memory_Profiler::Get_profiled_processes_list()const{

    return Profiled_processes;
}*/

map<pid_t,Process_handler> Memory_Profiler::Get_all_processes_list()const{

    return Processes;
}


inline void Memory_Profiler::Send_signal_to_process(pid_t PID){

    map<pid_t,Process_handler>::iterator it = Processes.find(PID);

    it->second.Send_signal();

}

void Memory_Profiler::Send_signal_to_all_processes(){

    map<pid_t,Process_handler>::iterator it;

    for(it = Processes.begin(); it != Processes.end(); it++){

        if(it->second.Get_profiled() == true) {
            Send_signal_to_process(it->first);
        }
    }
}

void Memory_Profiler::Read_FIFO(){

    int mem_prof_fifo = open(fifo_path, O_RDONLY);
    char buffer[5];
    size_t buff_size = 5;
    pid_t pid;
    int res;

        if(mem_prof_fifo != -1) {
            while((res = read(mem_prof_fifo,&buffer, sizeof(buffer))) != 0){

                if(res != -1){
                    pid = std::stoi(buffer,&buff_size);
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


void  Memory_Profiler::Print_profiled_process_shared_memory(pid_t PID){

    map<pid_t,Process_handler>::iterator it = Processes.find(PID);

    if(it->second.Get_profiled() == true){
        malloc_struct_t *shared_memory = it->second.Get_shared_memory();
        cout << "Shared memory PID: " << PID << " len: " << shared_memory->len << endl;
    }

}


void  Memory_Profiler::Print_all_profiled_processes_shared_memory(){

    cout << "Printing shared memories" << endl;
    map<pid_t,Process_handler>::iterator it;

    for(it = Processes.begin(); it != Processes.end(); it++){

            Print_profiled_process_shared_memory(it->first);
        }
}

/*Process_handler::Process_handler(){

		//Init_shared_memory();

}*/

Process_handler::Process_handler(pid_t PID){

        this->PID = PID;
        this->profiled = false;
        this->malloc_struct = NULL;
        this->shared_memory = 0;

}

Process_handler::Process_handler(const Process_handler &obj){

	cout << "copy constructor" << endl;

	this->PID = obj.PID;
	this->profiled = obj.profiled;
	this->malloc_struct = NULL;
	this->shared_memory = 0;

	this->Init_shared_memory();

}

Process_handler::~Process_handler(){

    munmap(malloc_struct,sizeof(malloc_struct_t));

}

void Process_handler::Init_shared_memory(){

    char PID_string[5];

    sprintf(PID_string,"%d",this->PID);

    this -> shared_memory = shm_open(PID_string, O_RDONLY , S_IRWXU | S_IRWXG | S_IRWXO);
    if(shared_memory < 0){

        printf("Error while creating shared memory:%d \n",errno);

    }

    this->malloc_struct = (malloc_struct_t*)mmap(NULL,sizeof(malloc_struct_t),PROT_READ,MAP_SHARED,this->shared_memory,0);

    if(malloc_struct == MAP_FAILED) {

        printf("Failed mapping the shared memory: %d \n",errno);

    }
}


void Process_handler::Send_signal(){

    kill(this->PID,SIGUSR1);

}


malloc_struct_t* Process_handler::Get_shared_memory(){

    return this->malloc_struct;
}

void* Read_FIFO_thread(void *arg) {

    while(true){
    mem_prof.Read_FIFO();

    sleep(1);
    }

    return 0;
}

int main()
{

    int err = pthread_create(&FIFO_read_thread_id, NULL, &Read_FIFO_thread, NULL);
        if(err){
            printf("Thread creation failed error:%d \n",err);
        }
        else {
            printf("Thread created\n");
        }


    while(1){

        getchar();
        //mem_prof.Print_all_processes();
        mem_prof.Add_all_process_to_profiling();
        mem_prof.Print_profiled_processes();
        cout << "Added all to profiled" << endl;
        mem_prof.Send_signal_to_all_processes();
        cout << "Signal sent" << endl;
        getchar();
        mem_prof.Print_all_profiled_processes_shared_memory();
        //sleep(3);
    }

    return 0;
}
