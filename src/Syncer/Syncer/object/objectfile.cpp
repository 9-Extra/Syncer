#include "objectfile.h"

namespace Syncer {

namespace fs = std::filesystem;

FileObject FileObject::open(const fs::path &path) {
    DataChunk content = read_whole_file(open_file_read(path).handle);
    FileHead *head = (FileHead *)content.start;
    if (content.size < sizeof(FileHead) || std::string_view((char *)content.start, 4) != "sync") {
        throw SyncerException("无效Object");
    }
    FileObject object;
    switch ((ObjectType)head->object_type) {
    case REGULAR_FILE: {
        FileInfo &info = object.data.emplace<FileInfo>();
        const void *ptr = fill_struct(info.attribute, (char *)content.start + sizeof(FileHead));
        info.content = DataChunk(content.size - sizeof(FileHead) - sizeof(FILE_BASIC_INFO));
        memcpy(info.content.start, ptr, info.content.size);
        break;
    }
    case SYMLINK: {
        Symlink &link = object.data.emplace<Symlink>();
        const void *ptr = fill_struct(link.attribute, (char *)content.start + sizeof(FileHead));
        wchar_t *str_start = (wchar_t *)ptr;
        wchar_t *str_end = (wchar_t *)((char *)content.start + content.size);
        link.target = fs::path(std::wstring(str_start, str_end + 1));
        break;
    }
    case DIRECTORY: {
        Directory &dir = object.data.emplace<Directory>();
        void *ptr = fill_struct(dir.attribute, (char *)content.start + sizeof(FileHead));
        uint32_t entry_count;
        ptr = fill_struct(entry_count, ptr);

        break;
    }
    default: {
        throw SyncerException("无效Object");
    }
    }

    return object;
}
void FileObject::write(std::ostream &stream) {
    assert(data.index() != 0);
    // FileHead
    stream.write("sync", 4);
    int object_type = (int)data.index();
    stream.write((char *)&object_type, sizeof(int));
    switch ((ObjectType)object_type) {
    case REGULAR_FILE: {
        const FileInfo &info = std::get<FileInfo>(data);
        stream.write((char *)&info.attribute, sizeof(info.attribute));
        stream.write((char *)info.content.start, info.content.size);
        break;
    }
    case SYMLINK: {
        Symlink &link = std::get<Symlink>(data);
        stream.write((char *)&link.attribute, sizeof(link.attribute));
        stream << link.target;
        break;
    }
    case DIRECTORY: {
        Directory &dir = std::get<Directory>(data);
        stream.write((char *)&dir.attribute, sizeof(dir.attribute));

        break;
    }
    default: {
        throw SyncerException("无效Object");
    }
    }
}
} // namespace Syncer