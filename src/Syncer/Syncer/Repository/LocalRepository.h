#pragma once
#include "Repository.h"
#include "../object/objectfile.h"
#include <unordered_map>

namespace Syncer {

namespace fs = std::filesystem;

class LocalRepository : public Repository {
public:
    LocalRepository(const fs::path &root, const fs::path &storage_path) : Repository(root), storage_path(storage_path) {
        try {
            if (fs::exists(storage_path) && !fs::is_directory(storage_path)) {
                throw RepositoryException(std::format("目标文件已存在，无法创建目录 {}", storage_path.string()));
            }
            fs::create_directories(storage_path);
        } catch (const fs::filesystem_error &e) {
            throw RepositoryException(std::format("打开仓库失败: {}", e.what()));
        }

        fs::path root_dir_file = storage_path / "root.obj";
        if (fs::is_regular_file(root_dir_file)){
            root_object = FileObject::open(root_dir_file);
        } else {
            root_object = FileObject::build_empty_directory(FILE_BASIC_INFO());
            std::ofstream out(root_dir_file);
            root_object.write(out);
            out.close();
        }

    }

    virtual void update_file(const fs::path &relative_path) override {
        fs::path standard_path = relative_path.lexically_normal();
        fs::path abs_path = root / standard_path;
        if (!fs::exists(abs_path)){
            throw RepositoryException(std::format("找不到文件: {}", abs_path.string()));
        }
        FileObject file = FileObject::open(abs_path);
        entry_list.emplace(standard_path, std::move(file));
        
    }
    virtual void remove_file(const fs::path &standard_path) override {}
    virtual void sync(const fs::path &relative_path) override {}

private:
    fs::path storage_path; // 存储路径
    //为了方便即时更新，存储从路径->文件元数据的映射
    std::unordered_map<fs::path, FileObject> entry_list;
    FileObject root_object;

};

} // namespace Syncer