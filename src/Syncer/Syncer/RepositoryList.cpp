#include "RepositoryList.h"
#include "base/winapi.h"
#include <iostream>
#include <fstream>
#include "base/SyncerException.h"

namespace Syncer {

RepositoryList repository_list;
fs::path json_file_path;
void init_json_file_path() {
    wchar_t buffer[1024];
    GetModuleFileNameW(NULL, buffer, 1023);
    json_file_path = fs::path(buffer).parent_path() / "resp_list.json";
    std::cout << json_file_path << std::endl;
}
void load_config_file() {
    std::ifstream config_file(json_file_path);
    if (!config_file) {
        throw Syncer::SyncerException(std::format("打开仓库文件文件: {} 失败", json_file_path.string()));
    }
    try {
        nlohmann::json json = nlohmann::json::parse(config_file);
        json.get_to(repository_list);
    } catch (const nlohmann::json::exception &e) {
        throw SyncerException(std::format("Json 文件格式错误: {}", e.what()));
    }
}
void save_config_file() {
    std::ofstream config_file(json_file_path);
    if (!config_file) {
        throw Syncer::SyncerException(std::format("创建仓库文件文件: {} 失败", json_file_path.string()));
    }
    config_file << nlohmann::json(repository_list).dump();
}
} // namespace Syncer