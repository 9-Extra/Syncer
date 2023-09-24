#include <filesystem>
#include "base/SyncerException.h"
namespace Syncer {

namespace fs = std::filesystem;

void init_backup_system();
void stop_backup_system();
struct FilterDesc{
    //todo
};

struct AutoBackupDesc{
    unsigned int interval;
};
struct RepositoryDesc{
    std::string name;
    fs::path source_path;
    fs::path target_path;
    FilterDesc filter;

    bool do_encryption;
    std::string password;

    bool do_packup;//以打包的形式备份
    bool enable_autobackup;
    AutoBackupDesc auto_backup_config; 
};

uint32_t register_repository(const RepositoryDesc& desc, bool immedate_backup=true);

struct RepositoryInfo{
    uint32_t id;
    fs::path source_path;
    fs::path target_path;
    FilterDesc filter;

    uint32_t file_number;
    bool need_password;

    bool packup;
    bool enable_autobackup;
    AutoBackupDesc auto_backup_config;
    std::string last_backup_time; 
};

std::vector<RepositoryInfo> list_repository();

void delete_repository(uint32_t id);

struct RecoverConfig{
    std::string password;
    enum {

    } config;
};
void recover_repository(uint32_t id);


}