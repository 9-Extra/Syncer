#include "AutoBackup.h"

#include "RepositoryList.h"
#include "Syncer/base/log.h"

namespace Syncer {
AutoBackupManager autobackup_manager;

void AutoBackupManager::backup_loop_func() {
    LOG_DEBUG("备份线程已启动");
    while (!thread_stop_flag) {
        SyTimePoint next_wakeup_point = SyTimePoint::max();
        try {
            LOG_TRACE("备份循环执行");
            auto repos = repository_list.get_resp_list();
            std::unique_lock lock(repository_list.repo_lock, std::adopt_lock_t());
            for (auto &[uuid, rep] : repos) {
                if (rep.do_autobackup) {
                    SyTimePoint next_backup_time =
                        rep.autobackup_config.last_backup_time + std::chrono::seconds(rep.autobackup_config.interval);
                    if (next_backup_time <= SyTimePoint::clock::now()) {
                        LOG_DEBUG("执行自动备份: {}", rep.uuid);
                        try {
                            repository_list.do_backup(rep);
                        } catch (const std::exception &e){
                            LOG_WARN("自动备份{}时出现异常，自动备份仍将继续运行", rep.uuid, e.what());
                        }
                        next_backup_time = rep.autobackup_config.last_backup_time +
                                                std::chrono::seconds(rep.autobackup_config.interval);
                    }
                    next_wakeup_point = std::min(next_wakeup_point, next_backup_time);
                }
            }

            repository_list.save_config_file(); // 写入更新的备份时间到文件中

        } catch (const std::exception &e) {
            LOG_ERROR("备份线程出现异常: ", e.what());
        }

        std::unique_lock lock(sleep_mutex);
        sleep_condition.wait_until(lock, next_wakeup_point);
    }

    LOG_DEBUG("备份线程退出");
}
} // namespace Syncer