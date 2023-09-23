#pragma once
#include <thread>
#include <assert.h>

namespace Syncer {
    extern std::thread backup_thread;
    
    void start_auto_backup();
    void stop_auto_backup();
}