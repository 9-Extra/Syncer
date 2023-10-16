#include <iostream>
#include <Syncer/syncer.h>

int main() {
    std::cout.sync_with_stdio(false);
    try {

        Syncer::init_backup_system();

        Syncer::RepositoryDesc desc;
        desc.source_path = R"(C:\Users\77313\Desktop\Sour式鏡音リンVer.2.01)";
        desc.target_path = R"(C:\Users\77313\Desktop\111111111\1.pack)";
        desc.do_encryption = false;
        desc.do_packup = true;
        desc.custom_name = "test";
        desc.enable_autobackup = true;
        desc.auto_backup_config.interval = 100;

        //std::string uuid = Syncer::register_repository(desc, true);
        //Syncer::recover_repository(uuid);

        for (auto& i : Syncer::list_repository()){
            std::cout << i.last_backup_time << std::endl;
        }

        Syncer::stop_backup_system();

    } catch (const Syncer::SyncerException &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    std::cout << "end\n";

    return 0;
}