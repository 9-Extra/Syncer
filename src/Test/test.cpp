#include <Syncer/syncer.h>
#include <iostream>
#include <thread>


int main() {
    std::cout.sync_with_stdio(false);

    // 启动
    Syncer::init_backup_system();

    {
        char error_reason[512]; // 保存错误信息

        // 获取现有所有仓库信息
        Syncer::LISTHANDLE *handle;
        if (!Syncer::list_repository_info(&handle)){
            Syncer::get_error_reason(error_reason, 512);
            std::cerr << error_reason << std::endl;
            return -1;
        }

        // 输出所有仓库的信息并且删除
        Syncer::RepositoryInfo info;
        for(size_t i = 0; i < Syncer::get_repository_list_size(handle);i++){
            Syncer::get_repository_info(handle, i, &info);

            std::cout << "仓库名: " << info.custom_name << std::endl;
            std::cout << "上一次备份时间: " << info.last_backup_time << std::endl;

            if (!Syncer::delete_repository(info.uuid)){
                Syncer::get_error_reason(error_reason, 512);
                std::cerr << error_reason << std::endl;
                return -1;
            }
        }
        
        // 释放handle
        Syncer::close_list_handle(handle);

        // 注册新的仓库
        Syncer::RepositoryDesc desc;
        desc.immedate_backup = true;
        desc.source_path = R"(C:\Users\77313\Desktop\Sour式鏡音リンVer.2.01)";
        desc.target_path = R"(C:\Users\77313\Desktop\111111111\1.pack)";
        desc.encrypt_method = "ks256";
        desc.password = "123456";
        desc.do_packup = false;
        desc.custom_name = "test";
        desc.enable_autobackup = false;
        desc.auto_backup_config.interval = 5;

        char uuid[40];
        if (!Syncer::register_repository(&desc, uuid)){
            Syncer::get_error_reason(error_reason, 512);
            std::cerr << error_reason << std::endl;
            return -1;
        }

        // 进行一次恢复
        if (!Syncer::recover_repository(uuid, "123456")){
            Syncer::get_error_reason(error_reason, 512);
            std::cerr << error_reason << std::endl;
            return -1;
        }

        // 等待，这是为了测试自动备份线程
        std::this_thread::sleep_for(std::chrono::seconds(100));
    }

    // 关闭系统，这会停止自动备份线程
    Syncer::stop_backup_system();

    std::cout << "end\n";

    return 0;
}