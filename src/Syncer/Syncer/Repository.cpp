#include "Repository.h"

namespace Syncer {

RepositoryAccessor::RepositoryAccessor(const fs::path &root) : root(root) {
    try {
        fs::create_directories(root);
    } catch (const fs::filesystem_error &e) {
        throw Syncer::SyncerException(std::format("打开仓库失败: {}", e.what()));
    }
    // 试图读取可能存在的目录文件
    {
        std::ifstream file_list_file(root / "file_list.res", std::ios_base::in | std::ios_base::binary);
        if (file_list_file) {
            if (!file_list.ParseFromIstream(&file_list_file)) {
                std::cout << "打开现有目录文件失败，将覆盖\n";
                file_list.Clear();
                is_dirty = true;
            } else {
                std::cout << "找到已有仓库\n";
            }
        } else {
            std::cout << "未找到仓库，新建\n";
            is_dirty = true;
        }
    }
}
void RepositoryAccessor::flush() {
    assert(file_list.IsInitialized()); // 检查完整性
    if (is_dirty) {
        {
            std::ofstream out(root / "file_list.res", std::ios_base::out | std::ios_base::binary | std::ios::trunc);
            std::cout << "Writing resp to " << root / "file_list.res";
            if (!out) {
                throw Syncer::SyncerException("写目录文件失败");
            }

            if (!file_list.SerializeToOstream(&out)) {
                throw Syncer::SyncerException("写目录文件失败");
            }
        }

        is_dirty = false;
    }
}
void RepositoryAccessor::update_file(const fs::path &standard_path, const FILE_BASIC_INFO &attibute,
                                     const Syncer::DataChunk &content) {}
} // namespace Syncer