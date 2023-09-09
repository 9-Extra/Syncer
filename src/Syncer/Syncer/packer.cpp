#include "packer.h"

namespace Syncer {

FILE_ID_INFO get_file_identity(HANDLE file) {
    FILE_ID_INFO info;
    if (!GetFileInformationByHandleEx(file, FILE_INFO_BY_HANDLE_CLASS::FileIdInfo, &info, sizeof(FILE_ID_INFO))) {
        throw Syncer::SyncerException("获取文件ID (GetFileInformationByHandleExFileIdInfo)失败");
    }

    return info;
}

struct FileInfo {
    Syncer::HandleWrapper handle;
    FILE_BASIC_INFO attibute; // 需要使用LastWirteTime来判断是否需要更新
    fs::path standard_path;
};
static std::unordered_map<FILE_ID_INFO, FileInfo> load_file_list(const fs::path &root, const FileFiliter &filiter) {
    std::unordered_map<FILE_ID_INFO, FileInfo> file_list;
    // filter所有文件，将通过的插入file_list
    try {
        for (fs::recursive_directory_iterator d_iter(root); auto &d : d_iter) {
            fs::path standard_path = d.path().lexically_relative(root);
            if (filiter.filiter_path(standard_path)) {
                fs::path abs_path = root / standard_path;
                Syncer::HandleWrapper handle = Syncer::open_file_read(abs_path);
                if (handle.is_vaild()) {
                    FILE_ID_INFO info;
                    if (!GetFileInformationByHandleEx(handle.handle, FileIdInfo, &info, sizeof(FILE_ID_INFO))) {
                        throw Syncer::SyncerException(std::format("获取文件ID失败: {}", abs_path.string()));
                    }
                    if (!file_list.contains(info)) {
                        FileInfo file;
                        file.handle = std::move(handle);
                        if (!GetFileInformationByHandleEx(file.handle.handle, FileBasicInfo, &file.attibute,
                                                          sizeof(FILE_BASIC_INFO))) {
                            throw Syncer::SyncerException(std::format("获取文件属性失败: {}", abs_path.string()));
                        }
                        if (filiter.filiter_attribute(standard_path, file.attibute)) {
                            file.standard_path = std::move(standard_path);
                            file_list[info] = std::move(file);
                        }
                    }
                }
            }
        }
    } catch (const fs::filesystem_error &e) {
        throw Syncer::SyncerException(e.what());
    }

    return file_list;
}

void pack(const fs::path &root, const fs::path &target, const FileFiliter &filiter) {
    uint32_t updated_file_count = 0;
    uint32_t totol_file_count = 0;

    std::unordered_map<FILE_ID_INFO, FileInfo> file_list = load_file_list(root, filiter);

    for (auto &[id, file] : file_list) {
        std::cout << file.standard_path << std::endl;
    }

    try {
        fs::create_directories(target);

    } catch (const fs::filesystem_error &e) {
        throw Syncer::SyncerException(e.what());
    }
}
} // namespace Syncer