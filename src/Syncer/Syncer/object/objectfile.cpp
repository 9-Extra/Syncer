#include "objectfile.h"
#include "../base/sha1.hpp"

namespace Syncer {

namespace fs = std::filesystem;

static void write_string(const std::string &str, std::ostream &stream) { stream << str << '\0'; }

static std::string load_string(const char **ptr) {
    const char *start = *ptr;
    for (; **ptr != '\0'; (*ptr)++)
        ;
    return std::string(start, (*ptr)++);
}

FileObject FileObject::open(const DataSpan &span){
    FileHead *head = (FileHead *)span.start;
    if (span.size < sizeof(FileHead) || std::string_view((char *)span.start, 4) != "sync") {
        throw SyncerException("无效Object");
    }
    FileObject object;
    const char *ptr = (char *)fill_struct(object.attribute, (char *)span.start + sizeof(FileHead));

    object.standard_path = load_string(&ptr);

    switch ((ObjectType)head->object_type) {
    case REGULAR_FILE: {
        FileInfo &info = object.data.emplace<FileInfo>();
        uint32_t hard_link_ref_count;
        ptr = (char *)fill_struct(hard_link_ref_count, ptr);
        for (uint32_t i = 0; i < hard_link_ref_count; i++) {
            info.hard_link_paths.emplace_back(load_string(&ptr));
        }
        info.content = DataChunk((char *)span.start + span.size - ptr);
        memcpy(info.content.start, ptr, info.content.size);
        break;
    }
    case SYMLINK: {
        Symlink &link = object.data.emplace<Symlink>();
        wchar_t *str_start = (wchar_t *)ptr;
        wchar_t *str_end = (wchar_t *)((char *)span.start + span.size);
        link.target = fs::path(std::wstring(str_start, str_end + 1));
        break;
    }
    case DIRECTORY: {
        object.data.emplace<Directory>();
        break;
    }
    default: {
        throw SyncerException("无效Object");
    }
    }

    return object;


}

FileObject FileObject::open(const fs::path &path) {
    DataChunk content = read_whole_file(open_file_read(path).handle);
    return open(DataSpan::from_chunk(content));
}

std::string FileObject::sha1() {
    SHA1 sha1;
    sha1.update(standard_path.string());
    return sha1.final();
}

void FileObject::write(std::ostream &stream) {
    assert(data.index() != 0);
    // FileHead
    stream.write("sync", 4);
    int object_type = (int)data.index();
    stream.write((char *)&object_type, sizeof(int));
    stream.write((char *)&attribute, sizeof(attribute));
    write_string(standard_path.string(), stream);
    switch ((ObjectType)object_type) {
    case REGULAR_FILE: {
        const FileInfo &info = std::get<FileInfo>(data);
        uint32_t hard_link_ref_count = info.hard_link_paths.size();
        stream.write((char*)&hard_link_ref_count, sizeof(hard_link_ref_count));
        for (uint32_t i = 0; i < hard_link_ref_count; i++) {
            write_string(info.hard_link_paths[i].string(), stream);
        }
        stream.write((char *)info.content.start, info.content.size);
        break;
    }
    case SYMLINK: {
        Symlink &link = std::get<Symlink>(data);
        stream << link.target;
        break;
    }
    case DIRECTORY: {
        break;
    }
    default: {
        throw SyncerException("无效Object");
    }
    }
}

DataChunk FileObject::serialize(){
    std::stringstream ss;
    write(ss);
    std::string_view view = ss.view();
    DataChunk chunk(view.size());
    memcpy(chunk.start, view.data(), view.size());
    return chunk;
}

void FileObject::recover(const fs::path &root) {
    assert(get_type() > NONE && get_type() < TYPE_COUNT);
    fs::path file_path = root / standard_path;
    fs::create_directories(file_path.parent_path());
    switch (get_type()) {
    case REGULAR_FILE: {
        const FileInfo &info = std::get<FileInfo>(data);
        write_whole_file_and_arrtibute(file_path, attribute, info.content);

        // 如果存在硬连接，其它硬连接链接到此文件
        for (size_t i = 1; i < info.hard_link_paths.size(); i++) {
            fs::path p = root / info.hard_link_paths[i];
            try {
                fs::create_hard_link(p, file_path);
            } catch (const fs::filesystem_error &e) {
                throw SyncerException(std::format("创建硬连接 {} 失败: {}", p.string(), e.what()));
            }
        }
        break;
    }
    case SYMLINK: {
        try {
            Symlink &link = std::get<Symlink>(data);
            fs::create_symlink(link.target, file_path);
            HandleWrapper handle(CreateFileW(file_path.wstring().c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                                             OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, NULL));
            if (!handle.is_vaild()) {
                throw SyncerException(std::format("打开符号连接 {} 失败: {}", file_path.string(), GetLastError()));
            }

            if (!SetFileInformationByHandle(handle.handle, FileBasicInfo, (void *)&attribute,
                                            sizeof(FILE_BASIC_INFO))) {
                throw SyncerException(std::format("设置符号连接 {} 属性失败: {}", file_path.string(), GetLastError()));
            }

        } catch (const fs::filesystem_error &e) {
            throw SyncerException(std::format("创建符号连接 {} 失败: {}", file_path.string(), e.what()));
        }
        break;
    }
    case DIRECTORY: {
        try {
            fs::create_directories(file_path);
        } catch (const fs::filesystem_error &e) {
            throw SyncerException(std::format("创建目录 {} 失败: {}", file_path.string(), e.what()));
        }
        HandleWrapper handle(CreateFileW(file_path.wstring().c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                                         OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                                         NULL));
        if (!handle.is_vaild()) {
            throw SyncerException(std::format("打开文件夹 {} 失败: {}", file_path.string(), GetLastError()));
        }

        if (!SetFileInformationByHandle(handle.handle, FileBasicInfo, (void *)&attribute, sizeof(FILE_BASIC_INFO))) {
            throw SyncerException(std::format("设置文件夹 {} 属性失败: {}", file_path.string(), GetLastError()));
        }
        break;
    }
    default:{
        assert(false);//unreachable
        break;
    }
    }
}
} // namespace Syncer