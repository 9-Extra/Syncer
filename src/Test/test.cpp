#include <Syncer/syncer.h>
#include <iostream>
#include <thread>


int main() {
    std::cout.sync_with_stdio(false);

    Syncer::init_backup_system();

    {
        char error_reason[512];
        Syncer::RepositoryDesc desc;
        desc.uuid = "7bc922bf-3e69-4b01-a229-5078287f324f";
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
        }
        if (!Syncer::recover_repository(uuid, "123456")){
            Syncer::get_error_reason(error_reason, 512);
            std::cerr << error_reason << std::endl;
        }

        Syncer::LISTHANDLE *handle;
        if (!Syncer::list_repository_info(&handle)){
            Syncer::get_error_reason(error_reason, 512);
            std::cerr << error_reason << std::endl;
        }

        Syncer::RepositoryInfo info;
        Syncer::get_repository_info(handle, 0, &info);
        std::cout << "仓库名: " << info.custom_name << std::endl;
        std::cout << "上一次备份时间: " << info.last_backup_time << std::endl;
        std::cout.flush();
        Syncer::close_list_handle(handle);

        std::this_thread::sleep_for(std::chrono::seconds(100));
    }

    Syncer::stop_backup_system();

    std::cout << "end\n";

    return 0;
}