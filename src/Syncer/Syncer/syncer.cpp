#include <Syncer/syncer.h>

#include "RepositoryList.h"
#include <iostream>
#include "packer.h"
#include "base/uuid.h"
namespace Syncer {
namespace fs = std::filesystem;

static void do_backup(RepositoryConfig& config){
    for(AutoBackupConfig& c : config.autobackup_list){
        c.last_backup_time = SyTimePoint::clock::now();
    }
    if (!config.do_packup){
        store(config.root, config.target_path, config.filter_desc);
    } else {
        pack(config.root, config.target_path, config.filter_desc);
    }
}

void init_backup_system(){
    init_json_file_path();
    if (fs::exists(json_file_path)){
        std::cout << "读取仓库文件" << std::endl;
        load_config_file();
    } else {
        std::cout << "创建新的仓库列表文件" << std::endl;
        save_config_file();
    }
}

void stop_backup_system(){
    

}

std::string register_repository(const RepositoryDesc& desc, bool immedate_backup){
    RepositoryConfig* ptr_config;
    if (desc.uuid.empty()){
        const std::string uuid = generate_guid_string();
        ptr_config = &repository_list.resp_list[uuid];
        ptr_config->uuid = uuid;
    } else {
        if (auto it = repository_list.resp_list.find(desc.uuid);it != repository_list.resp_list.end()){
            ptr_config = &it->second;
        } else {
            throw SyncerException("指定仓库不存在");
        }
    }
    
    RepositoryConfig& config = *ptr_config;
    config.custom_name = desc.custom_name;
    config.root = desc.source_path;
    config.target_path = desc.target_path;
    config.do_packup = desc.do_packup;
    if (desc.enable_autobackup){
        config.autobackup_list.clear();
        config.autobackup_list.emplace_back(desc.auto_backup_config.interval, SyTimePoint::min());
    }

    if (desc.do_encryption){
        config.encryption.method = "ks";
        config.encryption.key = desc.password;
    } else {
        config.encryption.method = "none";
    }

    // 设置完毕，检查有效性
    if (!config.do_packup){
        // todo
    } else {
        // todo
    }

    save_config_file();// 保存

    if (immedate_backup){
        do_backup(config);
    }

    return config.uuid;
}

std::vector<RepositoryInfo> list_repository(){
    std::vector<RepositoryInfo> result;
    for(const auto&[name, c]: repository_list.resp_list){
        RepositoryInfo info{
            .uuid = c.uuid,
            .source_path = c.root,
            .target_path = c.target_path,
            .filter = c.filter_desc,

            .file_number = 0,
            .need_password = c.encryption.method == "none" ? true : false,

            .packup = c.do_packup,
            .enable_autobackup = c.autobackup_list.size() != 0, 
        };

        if (info.enable_autobackup){
            if (c.autobackup_list[0].last_backup_time == SyTimePoint::min()){
                info.last_backup_time = "从未备份";
            } else {
                info.last_backup_time = to_loacltime(c.autobackup_list[0].last_backup_time);
            }
            info.auto_backup_config.interval = c.autobackup_list[0].interval;
        }

        result.emplace_back(info);
    }

    return result;
}

void immedately_backup_repository(const std::string& uuid){
    if (auto it = repository_list.resp_list.find(uuid);it != repository_list.resp_list.end()){
        do_backup(it->second);
    } else {
        throw Syncer::SyncerException("目标仓库不存在");
    }
}
void delete_repository(const std::string& uuid){
    if (auto it = repository_list.resp_list.find(uuid);it != repository_list.resp_list.end()){
        repository_list.resp_list.erase(it);
    } else {
        throw Syncer::SyncerException("目标仓库不存在");
    }
}

void recover_repository(const std::string &uuid) {
    if (auto it = repository_list.resp_list.find(uuid); it != repository_list.resp_list.end()) {
        RepositoryConfig& resp = it->second;
        std::cout << "还原仓库到:" << resp.root << std::endl;
        if (!resp.do_packup) {
            recover(resp.target_path, resp.root);
        } else {
            unpack(resp.target_path, resp.root);
        }
    } else {
        throw Syncer::SyncerException("目标仓库不存在");
    }
}
} // namespace Syncer