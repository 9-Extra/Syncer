#include "RepositoryList.h"
#include "base/winapi.h"
#include <iostream>
#include <fstream>
#include <Syncer/SyncerException.h>
#include "base/uuid.h"

namespace Syncer {

RepositoryList repository_list;
fs::path json_file_path;
void init_json_file_path() {
    wchar_t buffer[1024];
    GetModuleFileNameW(NULL, buffer, 1023);
    json_file_path = fs::path(buffer).parent_path() / "resp_list.json";
    std::cout << json_file_path << std::endl;
}
void RepositoryList::load_config_file() {
    std::ifstream config_file(json_file_path);
    if (!config_file) {
        throw Syncer::SyncerException(std::format("打开仓库文件文件: {} 失败", json_file_path.string()));
    }
    try {
        nlohmann::json json = nlohmann::json::parse(config_file);
        json.get_to(*this);
    } catch (const nlohmann::json::exception &e) {
        throw SyncerException(std::format("Json 文件格式错误: {}", e.what()));
    }
}
void RepositoryList::save_config_file() {
    std::ofstream config_file(json_file_path);
    if (!config_file) {
        throw Syncer::SyncerException(std::format("创建仓库文件文件: {} 失败", json_file_path.string()));
    }
    config_file << nlohmann::json(*this).dump();
}
void RepositoryList::register_repository(const RepositoryDesc *desc, char *uuid) {
    bool is_new;
    RepositoryConfig *ptr_config;
    if (desc->uuid == nullptr || desc->uuid[0] == '\0') {
        is_new = true;
        const std::string uuid = generate_guid_string();
        ptr_config = &resp_list[uuid];
        ptr_config->uuid = uuid;
    } else {
        if (auto it = resp_list.find(desc->uuid); it != resp_list.end()) {
            is_new = false;
            ptr_config = &it->second;
        } else {
            throw SyncerException("指定仓库不存在");
        }
    }

    RepositoryConfig &config = *ptr_config;
    config.custom_name = desc->custom_name;
    config.root = desc->source_path;
    config.target_path = desc->target_path;
    config.do_packup = desc->do_packup;
    config.do_autobackup = desc->enable_autobackup;
    if (config.do_autobackup){
        config.autobackup_config.interval = desc->auto_backup_config.interval;
        if (is_new){
            config.autobackup_config.last_backup_time = SyTimePoint::min();
        }
    }

    if (desc->do_encryption) {
        config.encryption.method = "ks";
        config.encryption.key = desc->password;
    } else {
        config.encryption.method = "none";
    }

    // 设置完毕，检查有效性
    if (!config.do_packup) {
        // todo
    } else {
        // todo
    }

    save_config_file(); // 保存

    if (desc->immedate_backup) {
        do_backup(config);
    }

    config.uuid.copy(uuid, config.uuid.size());
    uuid[config.uuid.size()] = '\0';
}
} // namespace Syncer