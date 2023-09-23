#include "syncer.h"

#include <format>
#include <fstream>
namespace Syncer {
namespace fs = std::filesystem;

void init_backup_system(){

}
void stop_backup_system(){

}

void register_repository(const RepositoryDesc& desc);

std::vector<RepositoryInfo> list_repository();

void delete_repository(uint32_t id);

void recover_repository(uint32_t id);
} // namespace Syncer