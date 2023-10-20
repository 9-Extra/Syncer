#include "AutoBackup.h"

#include "RepositoryList.h"

namespace Syncer {
AutoBackupManager autobackup_manager;

void AutoBackupManager::backup_loop_func() {
    while (!thread_stop_flag) {
        SyTimePoint next_wakeup_point = SyTimePoint::max();
        try {
            std::cout << "Backup Thread Loop" << std::endl;
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

        } catch (const std::exception &e) {
            std::cerr << "备份线程出现异常: " << e.what() << std::endl;
        }

        std::unique_lock lock(sleep_mutex);
        sleep_condition.wait_until(lock, next_wakeup_point);
    }

    std::cout << "Backup Thread exit" << std::endl;
}
} // namespace Syncer