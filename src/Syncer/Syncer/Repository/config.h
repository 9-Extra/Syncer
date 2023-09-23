#pragma once

#include <filesystem>
#include <string>
#include "../base/BackupTime.h"

namespace Syncer {

namespace fs = std::filesystem;

struct AutoBackupConfig{
    uint64_t interval;
    SyTimePoint last_backup_time;
};
struct RepositoryConfig {
    std::string name;
    fs::path root;
    fs::path target_path;

    std::vector<std::string> filter_list;

    std::string compression_method;

    struct EncryptionConfig{
        std::string method;
        std::string key;
    } encryption;

    std::vector<AutoBackupConfig> autobackup_list;
};

} // namespace Syncer
