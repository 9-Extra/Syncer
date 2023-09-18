#pragma once
#include "../base/SyncerException.h"
#include <filesystem>

namespace Syncer {

namespace fs = std::filesystem;

class RepositoryException : public Syncer::SyncerException {
public:
    explicit RepositoryException(const char *message) : Syncer::SyncerException(message) {}

    explicit RepositoryException(const std::string &message) : Syncer::SyncerException(message) {}
};

class Repository {
public:
    Repository(const fs::path &root) : root(root.lexically_normal()) {}
    Repository(const Repository &) = delete;

    virtual void update_file(const fs::path &relative_path) = 0; // 添加或者更新单个文件或者目录
    virtual void remove_file(const fs::path &standard_path) = 0; // 删除文件或者目录
    virtual void sync(const fs::path &relative_path) = 0;        // 与指定目录同步，可能是整个仓库

    const fs::path &get_root() const { return root; }

    virtual ~Repository() = default;

protected:
    fs::path root; // 要备份的文件夹路径
};

} // namespace Syncer