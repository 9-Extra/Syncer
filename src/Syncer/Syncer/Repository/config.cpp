#include "config.h"

#include "../Base/SyncerException.h"
#include <format>
#include <fstream>

namespace Syncer {

namespace fs = std::filesystem;

RepositoryConfig load_config(const fs::path &path) {
    RepositoryConfig config;

    std::ifstream config_file(path);
    if (!config_file.is_open()) {
        throw Syncer::SyncerException(std::format("打开文件: {} 失败", path.string()));
    }

    try {
        configor::json::value config_json = configor::json::parse(config_file);

        try {
            config.name = config_json["name"];
            const configor::json::value &content = config_json["content"];
            config.root = content["root"];
            if (!config.root.is_absolute()) {
                throw Syncer::SyncerException(std::format("仓库路径必须使用绝对路径"));
            }

            config.filter_list = content["ignore"];
            config.compression_method = content["compression"]["method"];
            {
                const auto& en = content["encryption"];
                config.encryption = {
                    .method = en["method"],
                    .key = en["key"]
                };
            } 

            for (const auto &j : content["autobackup"]) {
                const std::string &type = j["type"];
                if (type == "local") {
                    config.autobackup_list.emplace_back(
                        AutoBackupConfigLocal{.interval = j["interval"], .target_path = j["path"]});
                } else if (type == "remote") {
                    config.autobackup_list.emplace_back(
                        AutoBackupConfigRemote{.interval = j["interval"], .provider = j["provider"]});
                } else {
                    throw Syncer::SyncerException(std::format("不支持的自动备份方法: {}", type));
                }
            }

        } 
        catch (const configor::configor_exception &e) {
            throw Syncer::SyncerException(std::format("配置文件格式错误: {}", e.what()));
        }

    } catch (const configor::configor_exception &e) {
        throw Syncer::SyncerException(std::format("Json解析错误: {}", e.what()));
    }

    return config;
}

bool save_config(const RepositoryConfig &config, const fs::path &path) {

    std::ofstream config_file(path);

    return true;
}

} // namespace Syncer