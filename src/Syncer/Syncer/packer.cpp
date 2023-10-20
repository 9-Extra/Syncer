#include "packer.h"
#include "Syncer/base/log.h"
#include "object/objectfile.h"
#include "base/winapi.h"
#include <unordered_map>
#include "FileFiliter.h"
namespace Syncer {

FILE_ID_INFO get_file_identity(HANDLE file) {
    FILE_ID_INFO info;
    if (!GetFileInformationByHandleEx(file, FILE_INFO_BY_HANDLE_CLASS::FileIdInfo, &info, sizeof(FILE_ID_INFO))) {
        throw Syncer::SyncerException("获取文件ID (GetFileInformationByHandleExFileIdInfo)失败");
    }

    return info;
}

struct FileInfo {
    DataChunk content;
    FILE_BASIC_INFO attibute;            // 需要使用LastWirteTime来判断是否需要更新
    std::vector<fs::path> standard_path; // 硬连接具有多个path
};

struct DirectoryInfo {
    FILE_BASIC_INFO attibute;
    fs::path standard_path;
};

struct SymLinkInfo {
    FILE_BASIC_INFO attibute;
    fs::path standard_path;
    fs::path target_path;
};

struct DirectoryFiles {
    std::vector<FileInfo> files;
    std::vector<SymLinkInfo> symlinks;
    std::vector<DirectoryInfo> directorys;
};
static DirectoryFiles load_file_list(const fs::path &root, const FileFiliter& filiter) {
    DirectoryFiles result;
    std::unordered_map<FILE_ID_INFO, size_t> hard_link_map; // 硬连接判断(ID -> result.files中的下标)
    // filter所有文件，将通过的插入file_list
    try {
        for (fs::recursive_directory_iterator d_iter(root); auto &d : d_iter) {
            fs::path standard_path = d.path().lexically_relative(root);
            fs::path abs_path = root / standard_path;
            if (filiter.filiter_path(standard_path)) {
                switch (d.symlink_status().type()) {
                case std::filesystem::file_type::regular: {
                    Syncer::HandleWrapper handle = Syncer::open_file_read(abs_path);
                    FILE_ID_INFO info;
                    if (!GetFileInformationByHandleEx(handle.handle, FileIdInfo, &info, sizeof(FILE_ID_INFO))) {
                        throw Syncer::SyncerException(std::format("获取文件ID失败: {}", abs_path.string()));
                    }
                    if (const auto it = hard_link_map.find(info); it != hard_link_map.end()) {
                        // 硬连接存在，添加一条路径
                        result.files[it->second].standard_path.push_back(std::move(standard_path));
                    } else {
                        // 新文件，添加记录
                        FileInfo file;
                        if (!GetFileInformationByHandleEx(handle.handle, FileBasicInfo, &file.attibute,
                                                          sizeof(FILE_BASIC_INFO))) {
                            throw Syncer::SyncerException(std::format("获取文件属性失败: {}", abs_path.string()));
                        }
                        // 对属性进行过滤
                        if (filiter.filiter_attribute(standard_path, file.attibute)) {
                            file.standard_path.push_back(std::move(standard_path));//记录路径
                            file.content = read_whole_file(handle.handle);//记录内容

                            result.files.emplace_back(std::move(file));

                            hard_link_map[info] = result.files.size() - 1;
                        }
                    }

                    break;
                }
                case std::filesystem::file_type::directory: {
                    Syncer::HandleWrapper handle = Syncer::open_file_read(abs_path);

                    DirectoryInfo info;
                    if (!GetFileInformationByHandleEx(handle.handle, FileBasicInfo, &info.attibute,
                                                      sizeof(FILE_BASIC_INFO))) {
                        throw Syncer::SyncerException(std::format("获取文件属性失败: {}", abs_path.string()));
                    }
                    if (filiter.filiter_attribute(standard_path, info.attibute)) {
                        info.standard_path = std::move(standard_path);
                        result.directorys.push_back(std::move(info));
                    }
                    break;
                }
                case std::filesystem::file_type::symlink: {
                    SymLinkInfo info;
                    Syncer::HandleWrapper handle = Syncer::open_file_read(abs_path);
                    if (!GetFileInformationByHandleEx(handle.handle, FileBasicInfo, &info.attibute,
                                                      sizeof(FILE_BASIC_INFO))) {
                        throw Syncer::SyncerException(std::format("获取文件属性失败: {}", abs_path.string()));
                    }
                    if (filiter.filiter_attribute(standard_path, info.attibute)) {
                        info.standard_path = std::move(standard_path);
                        try {
                            info.target_path = fs::read_symlink(abs_path);
                        } catch (const fs::filesystem_error &e) {
                            throw SyncerException(std::format("读取软链接 {} 失败: {}", abs_path.string(), e.what()));
                        }
                        result.symlinks.push_back(std::move(info));
                    }
                    break;
                }
                default: {
                    throw SyncerException(std::format("不支持此文件类型 {}", standard_path.string()));
                }
                }
            }
        }
    } catch (const fs::filesystem_error &e) {
        throw Syncer::SyncerException(e.what());
    }

    return result;
}

void copy(const fs::path &root, const fs::path &target, const std::string &filiter) {
    // uint32_t updated_file_count = 0;
    // uint32_t totol_file_count = 0;

    const auto [file_list, symlink_list, directory_list] = load_file_list(root, filiter);

    try {
        fs::remove_all(target);
    } catch (const fs::filesystem_error &e) {
        throw SyncerException(std::format("删除 {} 失败: {}", target.string(), e.what()));
    }

    for (const auto &dir : directory_list) {
        fs::path abs_path = target / dir.standard_path;
        try {
            fs::create_directories(abs_path);
        } catch (const fs::filesystem_error &e) {
            throw SyncerException(std::format("创建目录 {} 失败: {}", abs_path.string(), e.what()));
        }
        HandleWrapper handle(CreateFileW(abs_path.wstring().c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                                         OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                                         NULL));
        if (!handle.is_vaild()) {
            throw SyncerException(std::format("打开文件夹 {} 失败: {}", abs_path.string(), GetLastError()));
        }

        if (!SetFileInformationByHandle(handle.handle, FileBasicInfo, (void *)&dir.attibute, sizeof(FILE_BASIC_INFO))) {
            throw SyncerException(std::format("设置文件夹 {} 属性失败: {}", abs_path.string(), GetLastError()));
        }
    }

    for (const auto &link : symlink_list) {
        fs::path abs_path = target / link.standard_path;
        try {
            fs::create_symlink(link.target_path, abs_path);
            HandleWrapper handle(CreateFileW(abs_path.wstring().c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                                             OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, NULL));
            if (!handle.is_vaild()) {
                throw SyncerException(std::format("打开符号连接 {} 失败: {}", abs_path.string(), GetLastError()));
            }

            if (!SetFileInformationByHandle(handle.handle, FileBasicInfo, (void *)&link.attibute,
                                            sizeof(FILE_BASIC_INFO))) {
                throw SyncerException(std::format("设置符号连接 {} 属性失败: {}", abs_path.string(), GetLastError()));
            }

        } catch (const fs::filesystem_error &e) {
            throw SyncerException(std::format("创建符号连接 {} 失败: {}", abs_path.string(), e.what()));
        }
    }

    for (const auto &info : file_list) {
        const DataChunk& file_content = info.content;
        const fs::path &p = info.standard_path[0];
        // 创建第一个文件
        fs::path first_file_path = target / p;
        write_whole_file_and_arrtibute(first_file_path, info.attibute, file_content);

        // 如果存在硬连接，其它硬连接链接到此文件
        for (size_t i = 1; i < info.standard_path.size(); i++) {
            try {
                fs::path p = target / info.standard_path[i];
                fs::create_hard_link(first_file_path, p);
            } catch (const fs::filesystem_error &e) {
                throw SyncerException(std::format("创建硬连接 {} 失败: {}", p.string(), e.what()));
            }
        }
    }
}

void store(const fs::path &root, const fs::path &target, const std::string &filiter, EncryptFactory::Encoder* encoder){
    assert(encoder);
    auto [file_list, symlink_list, directory_list] = load_file_list(root, FileFiliter(filiter));

    try {
        fs::remove_all(target);
    } catch (const fs::filesystem_error &e) {
        throw SyncerException(std::format("删除 {} 失败: {}", target.string(), e.what()));
    }

    std::vector<FileObject> pool;
    for (auto &dir : directory_list) {
        pool.emplace_back(FileObject::build_empty_directory(dir.standard_path, dir.attibute));
    }

    for (auto &link : symlink_list) {
        pool.emplace_back(FileObject::build_symlink(link.standard_path, link.attibute, link.target_path));
    }

    for (auto &info : file_list) {
        std::vector refs(info.standard_path.begin() + 1, info.standard_path.end());
        pool.emplace_back(FileObject::build_file(info.standard_path[0], info.attibute, refs, std::move(info.content)));
    }

    fs::create_directories(target);
    for(FileObject& o : pool){
        fs::path p = target / o.sha1();

        DataChunk encrypted = encoder->encode(o.serialize());
        encrypted.write_to_file(p);
    }
}

void recover(const fs::path &storage_path, const fs::path &target, EncryptFactory::Decoder* decoder){
    for(const fs::directory_entry& entry : fs::directory_iterator(storage_path)){
        try{
            DataChunk encrypted = read_whole_file(open_file_read(storage_path / entry.path()).handle);
            DataChunk decoded = decoder->decode(encrypted);
            FileObject o = FileObject::open(DataSpan::from_chunk(decoded));
            o.recover(target);
        } catch (const Syncer::SyncerException& e){
            LOG_ERROR("读取文件{}失败， 跳过。原因: {}", (storage_path / entry.path()).string(), e.what());
        }
    }
}

void pack(const fs::path &root, const fs::path &target, const std::string &filiter){
    auto [file_list, symlink_list, directory_list] = load_file_list(root, FileFiliter(filiter));
    fs::create_directories(target.parent_path());
    std::ofstream out_file(target, std::ios_base::out | std::ios_base::binary);
    if (!out_file){
        throw SyncerException(std::format("创建文件 {} 失败", target.string()));
    }

    std::vector<DataChunk> pool;
    for (auto &dir : directory_list) {
        pool.emplace_back(FileObject::build_empty_directory(dir.standard_path, dir.attibute).serialize());
    }

    for (auto &link : symlink_list) {
        pool.emplace_back(FileObject::build_symlink(link.standard_path, link.attibute, link.target_path).serialize());
    }

    for (auto &info : file_list) {
        std::vector refs(info.standard_path.begin() + 1, info.standard_path.end());
        pool.emplace_back(FileObject::build_file(info.standard_path[0], info.attibute, refs, std::move(info.content)).serialize());
    }

    uint64_t chunk_count = pool.size();
    out_file.write((char*)&chunk_count, sizeof(chunk_count));
    for(const DataChunk& c : pool){
        uint64_t chunk_size = c.size;
        out_file.write((char*)&chunk_size, sizeof(chunk_size));
    }
    for(const DataChunk& c : pool){
        out_file.write((char*)c.start, c.size);
    }
}

void unpack(const fs::path &pack_file, const fs::path &target){
    std::ifstream in_file(pack_file, std::ios_base::in | std::ios_base::binary);
    if (!in_file){
        throw SyncerException(std::format("打开文件 {} 失败", pack_file.string()));
    }
    
    std::vector<DataChunk> chunks;
    uint64_t chunk_count;
    in_file.read((char*)&chunk_count, sizeof(chunk_count));
    for(uint64_t i = 0; i < chunk_count;i++){
        uint64_t chunk_size;
        in_file.read((char*)&chunk_size, sizeof(chunk_size));
        chunks.emplace_back(chunk_size);
    }

    for(DataChunk& c : chunks){
        in_file.read((char*)c.start, c.size);
        FileObject o = FileObject::open(DataSpan::from_chunk(c));
        o.recover(target);
    }
}


} // namespace Syncer