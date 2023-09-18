#pragma once
#include "../file.h"
#include <Syncer/base/DataChunk.h>
#include <Syncer/base/winapi.h>
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <variant>


namespace Syncer {

namespace fs = std::filesystem;

class FileObject final {
public:
    enum ObjectType { NONE = 0, REGULAR_FILE = 1, SYMLINK = 2, DIRECTORY = 3, TYPE_COUNT };

    FileObject() = default;

    ObjectType get_type() const { return (ObjectType)data.index(); }

    ~FileObject() {}

    static FileObject open(const fs::path &path);

    static FileObject build_file(const FILE_BASIC_INFO &attribute, DataChunk&& content) {
        FileObject object;
        object.data.emplace<FileInfo>(attribute, std::forward<DataChunk>(content));
        return object;
    }


    void write(std::ostream &stream);

private:
    struct FileInfo {
        FILE_BASIC_INFO attribute;
        DataChunk content;
    };

    struct Symlink {
        FILE_BASIC_INFO attribute;
        fs::path target;
    };

    struct Directory {
        FILE_BASIC_INFO attribute;
        std::vector<std::string> children_uuid;
    };

    std::variant<std::monostate, FileInfo, Symlink, Directory> data;

    struct FileHead {
        uint8_t magic[4];
        int object_type;
    };
};

} // namespace Syncer