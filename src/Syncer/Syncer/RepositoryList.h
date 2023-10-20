#pragma once

#include "Repository/config.h"
#include "Syncer/base/log.h"
#include "packer.h"
#include <Syncer/syncer.h>
#include <mutex>

namespace Syncer {

struct RepositoryList {
    std::recursive_mutex repo_lock; // 仓库访问锁

    void load_config_file();
    void save_config_file();

    // 记得解锁
    std::unordered_map<std::string, RepositoryConfig>& get_resp_list(){
        repo_lock.lock();
        return resp_list;
    }

    friend void to_json(nlohmann::json &j, const RepositoryList &t) {
        j = nlohmann::json::array();
        for (const auto &[uuid, c] : t.resp_list) {
            j.emplace_back(c);
        }
    }
    friend void from_json(const nlohmann::json &j, RepositoryList &t) {
        for (auto &&it : j) {
            t.resp_list.emplace(it.at("uuid").get<std::string>(), it.get<RepositoryConfig>());
        }
    }

    static void do_backup(RepositoryConfig &config);

    void register_repository(const RepositoryDesc *desc, char *uuid);
    void immedately_backup_repository(const char *uuid) {
        std::unique_lock lock(repo_lock);
        if (auto it = resp_list.find(uuid); it != resp_list.end()) {
            do_backup(it->second);
        } else {
            throw SyncerException("目标仓库不存在");
        }
    }
    void recover_repository(const char *uuid, const std::string &password);

    void delete_repository(const char *uuid) {
        std::unique_lock lock(repo_lock);
        if (auto it = resp_list.find(uuid); it != resp_list.end()) {
            LOG_INFO("删除仓库: {}", it->second.uuid);
            fs::remove_all(it->second.target_path);
            resp_list.erase(it);
        } else {
            throw SyncerException("目标仓库不存在");
        }
    }
private:
    std::unordered_map<std::string, RepositoryConfig> resp_list;
};

extern fs::path json_file_path;
extern RepositoryList repository_list;

void init_json_file_path();
} // namespace Syncer