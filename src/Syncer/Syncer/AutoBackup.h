#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <assert.h>
#include "RepositoryList.h"
#include "packer.h"

namespace Syncer {

class AutoBackupManager{
public:
    void start(){
        thread_stop_flag = false;
        backup_thread = std::thread(backup_loop_func_wapper, this);
    }

    void wakeup_backupthread(){
        sleep_condition.notify_one();
    }

    void stop(){
        thread_stop_flag = true;
        sleep_condition.notify_one();
        if (backup_thread.joinable()){
            backup_thread.join();
        }
    }

    ~AutoBackupManager(){
        assert(!backup_thread.joinable());
    }

private:
    std::thread backup_thread;
    std::mutex sleep_mutex;
    std::condition_variable sleep_condition;

    volatile bool thread_stop_flag;

    void backup_loop_func();

    static void backup_loop_func_wapper(AutoBackupManager* ptr){
        ptr->backup_loop_func();
    }
};

extern AutoBackupManager autobackup_manager;

}