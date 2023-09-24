#pragma once

#include <filesystem>
#include <string>
#include "../base/BackupTime.h"
#include <nlohmann/json.hpp>

namespace Syncer {

namespace fs = std::filesystem;

struct AutoBackupConfig{
    uint64_t interval;
    SyTimePoint last_backup_time;
};

inline void to_json(nlohmann::json& j, const AutoBackupConfig& p) {
        j = nlohmann::json{{"interval", p.interval}, {"last_backup_time", p.last_backup_time.time_since_epoch().count()}};
    }

inline void from_json(const nlohmann::json& j, AutoBackupConfig& p) {
    j.at("interval").get_to(p.interval);
    uint64_t time_count = j.at("last_backup_time");
    p.last_backup_time = SyTimePoint(SyTimePoint::duration(time_count));
}
struct RepositoryConfig {
    std::string name;
    fs::path root;
    fs::path target_path;

    std::vector<std::string> filter_list;

    bool do_packup;//以打包的形式备份
    struct EncryptionConfig{
        std::string method;
        std::string key;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(EncryptionConfig, method, key);
    } encryption;

    std::vector<AutoBackupConfig> autobackup_list;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RepositoryConfig, name, root, target_path, filter_list, do_packup, encryption, autobackup_list);
};
} // namespace Syncer
