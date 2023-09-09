#pragma once

#include <configor/json.hpp>
#include <filesystem>
#include <string>
#include <variant>


namespace Syncer {

namespace fs = std::filesystem;

struct AutoBackupConfigLocal{
    uint64_t interval;
    fs::path target_path;
};

struct AutoBackupConfigRemote{
    uint64_t interval;
    std::string provider;
};


struct RepositoryConfig {
    std::string name;
    fs::path root;

    std::vector<std::string> filter_list;

    std::string compression_method;

    struct EncryptionConfig{
        std::string method;
        std::string key;
    } encryption;

    std::vector<std::variant<AutoBackupConfigLocal, AutoBackupConfigRemote>> autobackup_list;
};

} // namespace Syncer
