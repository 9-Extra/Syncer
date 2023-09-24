#include "syncer.h"

#include <format>
#include <fstream>
#include "RepositoryList.h"
#include "base/winapi.h"
#include <iostream>
#include "packer.h"
namespace Syncer {
namespace fs = std::filesystem;

static void do_backup(RepositoryConfig& config){
    for(AutoBackupConfig& c : config.autobackup_list){
        c.last_backup_time = SyTimePoint::clock::now();
    }
    pack(config.root, config.target_path, FileFiliter());
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
    save_config_file();
}

uint32_t register_repository(const RepositoryDesc& desc, bool immedate_backup){
    RepositoryConfig& config = repository_list.resp_list.emplace_back();
    config.name = desc.name;
    config.root = desc.source_path;
    config.target_path = desc.target_path;
    config.do_packup = desc.do_packup;
    if (desc.enable_autobackup){
        config.autobackup_list.emplace_back(desc.auto_backup_config.interval, SyTimePoint::min());
    }

    if (desc.do_encryption){
        config.encryption.method = "ks";
        config.encryption.key = desc.password;
    } else {
        config.encryption.method = "none";
    }

    if (immedate_backup){
        do_backup(config);
    }

    return repository_list.resp_list.size() - 1;
}

std::vector<RepositoryInfo> list_repository(){
    std::vector<RepositoryInfo> result;
    for(uint32_t i = 0;i < repository_list.resp_list.size();i++){
        RepositoryConfig& c = repository_list.resp_list[i];
        RepositoryInfo info{
            .id = i,
            .source_path = c.root,
            .target_path = c.target_path,
            .filter = FilterDesc(),

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

void delete_repository(uint32_t id){
    if (id >= repository_list.resp_list.size()){
        throw Syncer::SyncerException("目标仓库不存在");
    }
    std::swap(repository_list.resp_list[id], repository_list.resp_list.back());
    repository_list.resp_list.pop_back();
}

void recover_repository(uint32_t id){
    if (id >= repository_list.resp_list.size()){
        throw Syncer::SyncerException("目标仓库不存在");
    }
    do_backup(repository_list.resp_list[id]);
}
} // namespace Syncer