#pragma once

#include <filesystem>
#include "SyncerException.h"

namespace Syncer {

namespace fs = std::filesystem;

void init_backup_system();
void stop_backup_system();

struct AutoBackupDesc{
    unsigned int interval;
};
struct RepositoryDesc{
    std::string custom_name; // 用户自定义名字，可以留空
    fs::path source_path; // 要备份的文件夹路径
    fs::path target_path; // 存储的路径，必须是一个文件夹路径（不存在会自动创建），而对于打包必须是一个文件路径
    std::string filter; // 过滤器，未实现

    bool do_encryption; // 是否加密
    std::string password; // 密码

    bool do_packup;//以打包的形式备份
    bool enable_autobackup;
    AutoBackupDesc auto_backup_config; 
};

std::string register_repository(const RepositoryDesc& desc, bool immedate_backup=true);

struct RepositoryInfo{
    std::string uuid;
    fs::path source_path;
    fs::path target_path;
    std::string filter;

    uint32_t file_number;
    bool need_password;

    bool packup;
    bool enable_autobackup;
    AutoBackupDesc auto_backup_config;
    std::string last_backup_time; 
};

std::vector<RepositoryInfo> list_repository();

void immedately_backup_repository(const std::string& uuid);
void delete_repository(const std::string& uuid);

struct RecoverConfig{
    std::string password;
    enum {

    } config;
};
void recover_repository(const std::string& uuid);
    

}