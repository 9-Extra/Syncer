#include <Syncer/syncer.h>

#include "Autobackup/AutoBackup.h"
#include "Repository/RepositoryList.h"
#include "base/log.h"


namespace Syncer {
namespace fs = std::filesystem;

static std::string error_reason = "";

extern "C" {

bool init_backup_system() {
    try {
        logger.inititalize();
        init_json_file_path();
        EncryptFactory::init_encryptor_table();
        if (fs::exists(json_file_path)) {
            LOG_TRACE("读取仓库文件");
            repository_list.load_config_file();
        } else {
            LOG_TRACE("创建新的仓库列表文件");
            repository_list.save_config_file();
        }
        autobackup_manager.start();

    } catch (const std::exception &e) {
        error_reason = e.what();
        LOG_ERROR("{}抛出异常: {}", __PRETTY_FUNCTION__ ,e.what());
        return false;
    }
    return true;
}

bool stop_backup_system() {
    autobackup_manager.stop();
    repository_list.get_resp_list().clear();
    repository_list.repo_lock.unlock();
    logger.drop();
    return true;
}

bool get_error_reason(char *reason, size_t buffer_size) {
    if (buffer_size > error_reason.size()) {
        error_reason.copy(reason, error_reason.size());
        reason[error_reason.size()] = '\0';
    } else {
        error_reason.copy(reason, buffer_size - 1);
        reason[buffer_size] = '\0';
    }

    if (error_reason.empty()) {
        return false;
    } else {
        error_reason.clear();
        return true;
    }
}

bool register_repository(const RepositoryDesc *desc, char *uuid) {
    try {
        repository_list.register_repository(desc, uuid);
        autobackup_manager.wakeup_backupthread();
    } catch (const std::exception &e) {
        error_reason = e.what();
        LOG_ERROR("{}抛出异常: {}", __PRETTY_FUNCTION__ ,e.what());
        return false;
    }

    return true;
}

struct LISTHANDLE {
    struct Info {
        std::string uuid;
        std::string custom_name;
        fs::path source_path;
        fs::path target_path;
        std::string filter;
        std::string last_backup_time;

        uint32_t file_number;
        bool need_password;

        bool packup;
        bool enable_autobackup;
        AutoBackupDesc auto_backup_config;
    };

    std::vector<Info> info;
};
bool list_repository_info(LISTHANDLE **handle) {
    std::vector<LISTHANDLE::Info> result;
    try {
        auto list = repository_list.get_resp_list();
        std::unique_lock lock(repository_list.repo_lock, std::adopt_lock_t());
        for (const auto &[name, c] : list) {
            LISTHANDLE::Info &info = result.emplace_back();

            info.uuid = c.uuid;
            info.custom_name = c.custom_name;
            info.source_path = c.root;
            info.target_path = c.target_path;
            info.filter = c.filter_desc;

            info.file_number = 0;
            info.need_password = c.encryption.method == "none" ? true : false;

            info.packup = c.do_packup;
            info.enable_autobackup = c.do_autobackup;
            if (c.autobackup_config.last_backup_time == SyTimePoint::min()) {
                info.last_backup_time = "从未备份";
            } else {
                info.last_backup_time = to_loacltime(c.autobackup_config.last_backup_time);
            }

            if (info.enable_autobackup) {
                info.auto_backup_config.interval = c.autobackup_config.interval;
            }
        }

    } catch (const std::exception &e) {
        error_reason = e.what();
        *handle = nullptr;
        LOG_ERROR("{}抛出异常: {}", __PRETTY_FUNCTION__ ,e.what());
        return false;
    }

    *handle = new LISTHANDLE(std::move(result));

    return true;
}

bool list_repository_info_uuid(LISTHANDLE **handle, const char *uuid) {
    std::vector<LISTHANDLE::Info> result;
    try {
        auto list = repository_list.get_resp_list();
        std::unique_lock lock(repository_list.repo_lock, std::adopt_lock_t());
        if(auto it = list.find(uuid);it != list.end()) {
            RepositoryConfig& c = it->second;
            LISTHANDLE::Info &info = result.emplace_back();

            info.uuid = c.uuid;
            info.custom_name = c.custom_name;
            info.source_path = c.root;
            info.target_path = c.target_path;
            info.filter = c.filter_desc;

            info.file_number = 0;
            info.need_password = c.encryption.method == "none" ? true : false;

            info.packup = c.do_packup;
            info.enable_autobackup = c.do_autobackup;
            if (c.autobackup_config.last_backup_time == SyTimePoint::min()) {
                info.last_backup_time = "从未备份";
            } else {
                info.last_backup_time = to_loacltime(c.autobackup_config.last_backup_time);
            }

            if (info.enable_autobackup) {
                info.auto_backup_config.interval = c.autobackup_config.interval;
            }
        } else {
            throw SyncerException("指定仓库不存在");
        }

    } catch (const std::exception &e) {
        error_reason = e.what();
        LOG_ERROR("{}抛出异常: {}", __PRETTY_FUNCTION__ ,e.what());
        *handle = nullptr;
        return false;
    }

    *handle = new LISTHANDLE(std::move(result));

    return true;
}

size_t get_repository_list_size(LISTHANDLE *handle) { 
    if (handle == nullptr){
        LOG_WARN("handle无效");;
    }
    return handle == nullptr ? 0 : handle->info.size(); 
}

void get_repository_info(LISTHANDLE *handle, size_t index, RepositoryInfo *info) {
    if (handle == nullptr || index >= handle->info.size()) {
        memset(info, 0, sizeof(RepositoryInfo));
        LOG_WARN("试图删除无效handle或index过大");
        return;
    } else {
        const LISTHANDLE::Info &i = handle->info[index];
        info->uuid = i.uuid.c_str();
        info->custom_name = i.custom_name.c_str();
        info->source_path = i.source_path.string().c_str();
        info->target_path = i.target_path.string().c_str();
        info->filter = i.filter.c_str();
        info->last_backup_time = i.last_backup_time.c_str();

        info->file_number = i.file_number;
        info->need_password = i.need_password;

        info->packup = i.packup;
        info->enable_autobackup = i.enable_autobackup;
        info->auto_backup_config = i.auto_backup_config;
    }
}

void close_list_handle(LISTHANDLE *handle) { 
    if (handle != nullptr){
        delete handle;
    } else {
        LOG_WARN("试图删除无效handle");
    }
}

bool immedately_backup_repository(const char *uuid) {
    try {
        repository_list.immedately_backup_repository(uuid);
    } catch (const std::exception &e) {
        error_reason = e.what();
        LOG_ERROR("{}抛出异常: {}", __PRETTY_FUNCTION__ ,e.what());
        return false;
    }
    return true;
}

bool delete_repository(const char *uuid) {
    try {
        repository_list.delete_repository(uuid);
    } catch (const std::exception &e) {
        error_reason = e.what();
        LOG_ERROR("{}抛出异常: {}", __PRETTY_FUNCTION__ ,e.what());
        return false;
    }
    return true;
}

bool recover_repository(const char *uuid, const char *password){
    try {
        repository_list.recover_repository(uuid, password == nullptr ? "" : password);
    } catch (const std::exception &e) {
        error_reason = e.what();
        LOG_ERROR("{}抛出异常: {}", __PRETTY_FUNCTION__ ,e.what());
        return false;
    }
    return true;
}
}
} // namespace Syncer