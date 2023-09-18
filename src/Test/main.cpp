#include <Syncer/Repository/LocalRepository.h>
#include <Syncer/base/DataChunk.h>
#include <Syncer/packer.h>
#include <Syncer/syncer.h>
#include <configor/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>


namespace fs = std::filesystem;

int main() {
    std::cout.sync_with_stdio(false);
    try {

        // auto config = Syncer::load_config("syncer.json");
        // std::cout << "Root: " << config.root << std::endl << "Name: " << config.name << std::endl;

        // Syncer::pack(".", "C:/Users/77313/Desktop/新建文件夹/test", Syncer::FileFiliter());

        Syncer::LocalRepository resp(".", "./strage");
        FILE_BASIC_INFO info;
        Syncer::DataChunk content(100);
        const char str[] = "daowcjdowmacucdiowamj2132321";
        memcpy(content.start, str, sizeof(str));

        Syncer::FileObject file = Syncer::FileObject::build_file(info, std::move(content));
        std::ofstream out("./write.txt");
        file.write(out);
        out.close();

        Syncer::FileObject read = Syncer::FileObject::open("write.txt");

        std::cout << read.get_type() << std::endl;


    } catch (const Syncer::SyncerException &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    std::cout << "end\n";

    return 0;
}