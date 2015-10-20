#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>

#include "Memory_Profiler.h"


using namespace std;


int filter_func(const dirent * directory) {

    string name = directory->d_name;
    char* isNumber;

    strtol(name.c_str(),&isNumber,10);

    return (*isNumber == NULL);
}


Memory_Profiler::~Memory_Profiler() {

    for(int i =0; i<this->NumOfProcesses;i++){
            delete(this->ProcessesList[i]);
    }
    delete(this->ProcessesList);
    cout<< "Destructor" << endl;
}



void Memory_Profiler::Print_processes()const{

    if(ProcessesList){
        for(int i = 0; i< NumOfProcesses; i++){

            cout << ProcessesList[i]->d_name << endl;
        }
    }
    cout<< "Number of processes: " << NumOfProcesses << endl;
}

void Memory_Profiler::GetProcessList() {

    Memory_Profiler::NumOfProcesses = scandir("/proc", &(Memory_Profiler::ProcessesList),filter_func, NULL);
    return;
}


void Memory_Profiler::Add_Process(uint32_t PID) {

    Process_handler process(PID);
    //if(std::find(Profiled_Processes.begin(), Profiled_Processes.end(), process) == Profiled_Processes.end()) {  Operator overload kell Process_handler==Process_handler-re
        Profiled_Processes.push_back(process);
    //}

}

void Memory_Profiler::Add_all_Process(){


    for(int i = 0; i < NumOfProcesses; i++){

        Add_Process(std::stoul(ProcessesList[i]->d_name));

    }

    return;
}

std::vector<Process_handler> Memory_Profiler::Get_profiled_processes_list()const{

    return Memory_Profiler::Profiled_Processes;

}

void Memory_Profiler::Print_profiled_processes(){

    for(uint32_t i = 0; i< Profiled_Processes.size();i++){

    cout <<" PID is: "<< Profiled_Processes[i].GetPID() << endl;
    }
    return;
}

int main()
{

    Memory_Profiler mem_prof;

    mem_prof.GetProcessList();
    //mem_prof.Print_processes();

    /*mem_prof.Add_Process(8282);
    mem_prof.Add_Process(8349);*/

    mem_prof.Add_all_Process();


    mem_prof.Print_profiled_processes();



    return 0;
}
