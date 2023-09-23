#include "AutoBackup.h"

namespace Syncer {
    std::thread backup_thread;

    static void backup_thread_task(){
        assert(backup_thread.get_id() == std::this_thread::get_id());



    }

    void start_auto_backup(){
        backup_thread = std::thread(backup_thread_task);
    }
    void stop_auto_backup(){

    }
}