#include "RepositoryList.h"
#include "base/uuid.h"
#include "base/winapi.h"
#include <Syncer/SyncerException.h>
#include <fstream>
#include <iostream>

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
        {
            std::unique_lock lock(repo_lock);
            json.get_to(*this);
        }
    } catch (const nlohmann::json::exception &e) {
        throw SyncerException(std::format("Json 文件格式错误: {}", e.what()));
    }
}
void RepositoryList::save_config_file() {
    std::ofstream config_file(json_file_path);
    if (!config_file) {
        throw Syncer::SyncerException(std::format("创建仓库文件文件: {} 失败", json_file_path.string()));
    }
    {
        std::unique_lock lock(repo_lock);
        config_file << nlohmann::json(*this).dump();
    }
}
void RepositoryList::register_repository(const RepositoryDesc *desc, char *uuid) {
    bool is_new;
    std::unique_lock lock(repo_lock);
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
    auto factory = EncryptFactory::find_encryptor(desc->encrypt_method);
    if (factory == nullptr) {
        throw SyncerException(std::format("不支持加密算法 \"{}\"", desc->encrypt_method));
    }
    // 检查有效性
    // todo

    RepositoryConfig &config = *ptr_config;
    config.custom_name = desc->custom_name;
    config.root = desc->source_path;
    config.target_path = desc->target_path;
    config.do_packup = desc->do_packup;
    config.do_autobackup = desc->enable_autobackup;
    if (config.do_autobackup) {
        config.autobackup_config.interval = desc->auto_backup_config.interval;
        if (is_new) {
            config.autobackup_config.last_backup_time = SyTimePoint::min();
        }
    }

    config.encryption.method = desc->encrypt_method;
    config.encryption.key_hash = factory->generate_public_key(desc->password);

    save_config_file(); // 保存

    if (desc->immedate_backup) {
        do_backup(config);
    }

    config.uuid.copy(uuid, config.uuid.size());
    uuid[config.uuid.size()] = '\0';
}
void RepositoryList::recover_repository(const char *uuid, const std::string &password) {
    {
        std::unique_lock lock(repo_lock);
        if (auto it = resp_list.find(uuid); it != resp_list.end()) {
            RepositoryConfig &resp = it->second;
            std::cout << "还原仓库到:" << resp.root << std::endl;

            EncryptFactory *factory = EncryptFactory::find_encryptor(resp.encryption.method);
            if (factory == nullptr) {
                throw SyncerException(std::format("不支持加密算法 \"{}\"", resp.encryption.method));
            }
            if (resp.encryption.key_hash != factory->generate_public_key(password)) {
                throw SyncerException(std::format("密码不正确"));
            }

            if (!resp.do_packup) {
                auto decoder = factory->build_decoder(password);
                recover(resp.target_path, resp.root, decoder.get());
            } else {
                unpack(resp.target_path, resp.root);
            }
        } else {
            throw SyncerException("目标仓库不存在");
        }
    }
}
void RepositoryList::do_backup(RepositoryConfig &config) {
    std::cout << "正在备份：" << config.custom_name << std::endl;

    EncryptFactory *factory = EncryptFactory::find_encryptor(config.encryption.method);
    if (factory == nullptr) {
        throw SyncerException(std::format("不支持加密算法 {}", config.encryption.method));
    }
    if (!config.do_packup) {
        auto encoder = factory->build_encoder(config.encryption.key_hash);
        store(config.root, config.target_path, config.filter_desc, encoder.get());
    } else {
        pack(config.root, config.target_path, config.filter_desc);
    }
    if (config.do_autobackup) {
        config.autobackup_config.last_backup_time = SyTimePoint::clock::now();
    }
}
} // namespace Syncer