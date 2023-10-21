#include "RepositoryList.h"
#include "base/log.h"
#include "base/uuid.h"
#include "base/winapi.h"
#include <Syncer/SyncerException.h>
#include <fstream>

namespace Syncer {

RepositoryList repository_list;
fs::path json_file_path;
void init_json_file_path() {
    wchar_t buffer[1024];
    GetModuleFileNameW(NULL, buffer, 1023);
    json_file_path = fs::path(buffer).parent_path() / "resp_list.json";
    LOG_INFO("设置json文件路径{}", json_file_path.string());
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
    // 检查有效性
    auto factory = EncryptFactory::find_encryptor(desc->encrypt_method);
    if (factory == nullptr) {
        throw SyncerException(std::format("不支持加密算法 \"{}\"", desc->encrypt_method));
    }

    std::unique_lock lock(repo_lock);
    const std::string u = generate_guid_string();
    RepositoryConfig &config = resp_list[u];
    config.uuid = u;
    config.custom_name = desc->custom_name;
    config.root = desc->source_path;
    config.target_path = desc->target_path;
    config.do_packup = desc->do_packup;
    config.do_autobackup = desc->enable_autobackup;
    //config.autobackup_config.last_backup_time = SyTimePoint::min();
    if (config.do_autobackup) {
        config.autobackup_config.interval = desc->auto_backup_config.interval;
    }

    config.encryption.method = desc->encrypt_method;
    config.encryption.key_hash = factory->generate_public_key(desc->password);

    do_backup(config); // 立即进行一次备份
    save_config_file(); // 保存

    LOG_INFO("创建仓库{}完成", config.custom_name);

    config.uuid.copy(uuid, config.uuid.size());
    uuid[config.uuid.size()] = '\0';
}
void RepositoryList::recover_repository(const char *uuid, const std::string &password) {
    {
        std::unique_lock lock(repo_lock);
        if (auto it = resp_list.find(uuid); it != resp_list.end()) {
            RepositoryConfig &resp = it->second;
            LOG_INFO("还原仓库{}到: {}", resp.custom_name, resp.root.string());

            EncryptFactory *factory = EncryptFactory::find_encryptor(resp.encryption.method);
            if (factory == nullptr) {
                throw SyncerException(std::format("不支持加密算法 \"{}\"", resp.encryption.method));
            }
            if (resp.encryption.key_hash != factory->generate_public_key(password)) {
                throw SyncerException(std::format("密码不正确"));
            }

            auto decoder = factory->build_decoder(password);
            if (!resp.do_packup) {
                recover(resp.target_path, resp.root, decoder.get());
            } else {
                unpack(resp.target_path, resp.root, decoder.get());
            }
        } else {
            throw SyncerException("目标仓库不存在");
        }
    }
}
void RepositoryList::do_backup(RepositoryConfig &config) {
    LOG_INFO("正在备份仓库：{}", config.custom_name);

    try {
        EncryptFactory *factory = EncryptFactory::find_encryptor(config.encryption.method);
        if (factory == nullptr) {
            throw SyncerException(std::format("不支持加密算法 {}", config.encryption.method));
        }
        auto encoder = factory->build_encoder(config.encryption.key_hash);
        if (!config.do_packup) {
            store(config.root, config.target_path, config.filter_desc, encoder.get());
        } else {
            pack(config.root, config.target_path, config.filter_desc, encoder.get());
        }

        if (config.do_autobackup) {
            config.autobackup_config.last_backup_time = SyTimePoint::clock::now();
        }
    } catch (const SyncerException &e) {
        // 即使出现错误也要设置完成备份的时间
        if (config.do_autobackup) {
            config.autobackup_config.last_backup_time = SyTimePoint::clock::now();
        }
        throw e; // 原样抛出
    } 
}
} // namespace Syncer