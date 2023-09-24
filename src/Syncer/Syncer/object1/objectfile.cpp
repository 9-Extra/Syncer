#include "objectfile.h"
#include "../base/sha1.hpp"

namespace Syncer {

namespace fs = std::filesystem;

static void write_string(const std::string& str, std::ostream &stream){
    stream << str << '\0';
}

static std::string load_string(const char** ptr){
    const char* start = *ptr;
    for(;**ptr != '\0';(*ptr)++);
    return std::string(start, (*ptr)++);
}


FileObject FileObject::open(const fs::path &path) {
    DataChunk content = read_whole_file(open_file_read(path).handle);
    FileHead *head = (FileHead *)content.start;
    if (content.size < sizeof(FileHead) || std::string_view((char *)content.start, 4) != "sync") {
        throw SyncerException("无效Object");
    }
    FileObject object;
    const char *ptr = (char*)fill_struct(object.attribute, (char *)content.start + sizeof(FileHead));

    object.standard_path = load_string(&ptr);

    switch ((ObjectType)head->object_type) {
    case REGULAR_FILE: {
        FileInfo &info = object.data.emplace<FileInfo>();
        uint32_t hard_link_ref_count;
        ptr = (char*)fill_struct(hard_link_ref_count, ptr);
        for(uint32_t i = 0;i < hard_link_ref_count;i++){
            info.hard_link_paths.emplace_back(load_string(&ptr));
        }
        info.content = DataChunk((char*)content.start + content.size - ptr);
        memcpy(info.content.start, ptr, info.content.size);
        break;
    }
    case SYMLINK: {
        Symlink &link = object.data.emplace<Symlink>();
        wchar_t *str_start = (wchar_t *)ptr;
        wchar_t *str_end = (wchar_t *)((char *)content.start + content.size);
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

std::string FileObject::sha1(){
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
        for(uint32_t i = 0;i < hard_link_ref_count;i++){
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
} // namespace Syncer