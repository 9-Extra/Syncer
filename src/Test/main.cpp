#include <Syncer/base/DataChunk.h>
#include <Syncer/syncer.h>
#include <configor/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <Syncer/packer.h>
#include <Syncer/Repository.h>

namespace fs = std::filesystem;

int main() {
    std::cout.sync_with_stdio(false);
    try {

        // auto config = Syncer::load_config("syncer.json");
        // std::cout << "Root: " << config.root << std::endl << "Name: " << config.name << std::endl;

        // Syncer::pack(config.root, "./tar", Syncer::FileFiliter());

        Syncer::RepositoryAccessor resp("./resp");
        resp.flush();


    } catch (const Syncer::SyncerException &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return -1;
        }

        return 0;
}