
#include <iostream>

#include <fstream>
#include <string>

#include <mutex>
#include <future>

#include "common.h"
#include "FileWatcher.h"

using namespace std;

void monitoringDir(string dir, std::ofstream& log_out, mutex& log_file_mutex, bool& stop);

int main(int argc, char *argv[])
{
    if(argc < 2){
        cout << "No Extra Command Line Argument Passed";
        return 0;
    }

    string log_file_dir(argv[1]);

    std::ofstream log_out;
    log_out.open(log_file_dir + "/log_file_monitor.txt", ios::out | ios::app);
    mutex log_file_mutex;

    log_out << endl << "Start monitoring. Time: " << currentTime() << endl;

    bool stop = false;
    std::vector<future<void>> monitors;
    for(int i = 2; i < argc; i++){
        monitors.push_back(
            std::async(std::launch::async, 
                [&]{ monitoringDir(string(argv[i]) ,log_out, log_file_mutex, stop) }
                );
    }

    string command;
    while(!stop){
        cout << "Enter \"STOP\" for stop process" << endl;
        cin >> command;
        if(command == "STOP")
            stop = true;
    }

    for(auto process :monitors){
        process.wait();
    }
}

void monitoringDir(string dir, std::ofstream& log_out, mutex& log_file_mutex, bool& stop){

    wstring wDir(dir.begin(), dir.end());
    FileWacther watcher(wdir);

    while(!stop){
        
        auto result = watcher.nextChanges();
        if(!result)
            continue;
        
        std::lock_guard lock(log_file_mutex);
        
        for(auto changeMessage: result.value()){
            log_out << changeMessage << endl;
        }
    }

}