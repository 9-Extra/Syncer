#include <iostream>
#include <Syncer/syncer.h>

int main() {
    std::cout.sync_with_stdio(false);
    try {

        Syncer::init_backup_system();

        Syncer::RepositoryDesc desc;
        desc.source_path = R"(C:\Users\77313\Desktop\Sour式鏡音リンVer.2.01)";
        desc.target_path = R"(C:\Users\77313\Desktop\新建文件夹)";
        desc.do_encryption = false;
        desc.do_packup = false;
        desc.name = "test";
        desc.enable_autobackup = true;
        desc.auto_backup_config.interval = 100;

        Syncer::register_repository(desc, false);

        for (auto& i : Syncer::list_repository()){
            std::cout << i.last_backup_time << std::endl;
        }

        Syncer::stop_backup_system();
        // Syncer::LocalRepository resp(".", "./strage");
        // FILE_BASIC_INFO info;
        // Syncer::DataChunk content(100);
        // const char str[] = "daowcjdowmacucdiowamj2132321";
        // memcpy(content.start, str, sizeof(str));

        // Syncer::FileObject file = Syncer::FileObject::build_file(info, std::move(content));
        // std::ofstream out("./write.txt");
        // file.write(out);
        // out.close();

        // Syncer::FileObject read = Syncer::FileObject::open("write.txt");

        // std::cout << read.get_type() << std::endl;


    } catch (const Syncer::SyncerException &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    std::cout << "end\n";

    return 0;
}