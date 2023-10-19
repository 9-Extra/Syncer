#include "AutoBackup.h"

namespace Syncer{
    AutoBackupManager autobackup_manager;

    void AutoBackupManager::backup_loop_func() {
        while (!thread_stop_flag) {
            std::cout << "Backup Thread Loop" << std::endl;

            SyTimePoint next_wakeup_point = SyTimePoint::max();
            for (auto &[uuid, rep] : repository_list.resp_list) {
                if (rep.do_autobackup) {
                    SyTimePoint next_backup_time =
                        rep.autobackup_config.last_backup_time + std::chrono::seconds(rep.autobackup_config.interval);
                    if (next_backup_time <= SyTimePoint::clock::now()) {
                        repository_list.do_backup(rep);
                        next_wakeup_point = rep.autobackup_config.last_backup_time +
                                            std::chrono::seconds(rep.autobackup_config.interval);
                    }
                    next_wakeup_point = std::min(next_wakeup_point, next_backup_time);
                }
            }

            repository_list.save_config_file(); // 写入更新的备份时间到文件中

            std::unique_lock lock(sleep_mutex);
            sleep_condition.wait_until(lock, next_wakeup_point);
        }

        std::cout << "Backup Thread exit" << std::endl;
    }
    } // namespace Syncer