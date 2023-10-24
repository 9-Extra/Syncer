#pragma once

#include "../base/BackupTime.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

namespace Syncer {

namespace fs = std::filesystem;

struct AutoBackupConfig {
    uint64_t interval;
    SyTimePoint last_backup_time;
};

inline void to_json(nlohmann::json &j, const AutoBackupConfig &p) {
    j = nlohmann::json{{"interval", p.interval}, {"last_backup_time", p.last_backup_time.time_since_epoch().count()}};
}

inline void from_json(const nlohmann::json &j, AutoBackupConfig &p) {
    j.at("interval").get_to(p.interval);
    uint64_t time_count = j.at("last_backup_time");
    p.last_backup_time = SyTimePoint(SyTimePoint::duration(time_count));
}
struct RepositoryConfig {
    std::string uuid;
    std::u8string custom_name;
    fs::path root;
    fs::path target_path;

    std::string filter_desc;

    bool do_packup; // 以打包的形式备份
    struct EncryptionConfig {
        std::string method;
        std::string key_hash;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(EncryptionConfig, method, key_hash);
    } encryption;

    bool do_autobackup;
    AutoBackupConfig autobackup_config;

    friend void to_json(nlohmann ::json &nlohmann_json_j, const RepositoryConfig &nlohmann_json_t) {
        nlohmann_json_j["uuid"] = nlohmann_json_t.uuid;
        nlohmann_json_j["custom_name"] = (char*)nlohmann_json_t.custom_name.c_str();
        nlohmann_json_j["root"] = (char*)nlohmann_json_t.root.u8string().c_str();
        nlohmann_json_j["target_path"] = (char*)nlohmann_json_t.target_path.u8string().c_str();
        nlohmann_json_j["filter_desc"] = nlohmann_json_t.filter_desc;
        nlohmann_json_j["do_packup"] = nlohmann_json_t.do_packup;
        nlohmann_json_j["encryption"] = nlohmann_json_t.encryption;
        nlohmann_json_j["do_autobackup"] = nlohmann_json_t.do_autobackup;
        nlohmann_json_j["autobackup_config"] = nlohmann_json_t.autobackup_config;
    }
    friend void from_json(const nlohmann ::json &nlohmann_json_j, RepositoryConfig &nlohmann_json_t) {
        nlohmann_json_j.at("uuid").get_to(nlohmann_json_t.uuid);
        nlohmann_json_t.custom_name = (char8_t*)nlohmann_json_j.at("custom_name").get<std::string>().c_str();
        nlohmann_json_j.at("root").get_to(nlohmann_json_t.root);
        nlohmann_json_j.at("target_path").get_to(nlohmann_json_t.target_path);
        nlohmann_json_j.at("filter_desc").get_to(nlohmann_json_t.filter_desc);
        nlohmann_json_j.at("do_packup").get_to(nlohmann_json_t.do_packup);
        nlohmann_json_j.at("encryption").get_to(nlohmann_json_t.encryption);
        nlohmann_json_j.at("do_autobackup").get_to(nlohmann_json_t.do_autobackup);
        nlohmann_json_j.at("autobackup_config").get_to(nlohmann_json_t.autobackup_config);
    }
};
} // namespace Syncer
