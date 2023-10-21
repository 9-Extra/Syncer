#pragma once

#include "../base/winapi.h"
#include <filesystem>
#include <string>

namespace Syncer {

namespace fs = std::filesystem;

class FileFiliter {
public:
    FileFiliter() {}
    FileFiliter(const std::string &desc) {}

    bool filiter_path(const fs::path &path) const { return true; }
    bool filiter_attribute(const fs::path &path, const FILE_BASIC_INFO &attribute) const { return true; }
};
} // namespace Syncer