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

    FileObject(FileObject &&other) {
        standard_path = std::move(other.standard_path);
        attribute = other.attribute;
        data = std::move(other.data);
        other.data.emplace<std::monostate>();
    }

    FileObject &operator=(FileObject &&other) {
        standard_path = std::move(other.standard_path);
        attribute = other.attribute;
        data = std::move(other.data);
        other.data.emplace<std::monostate>();
        return *this;
    }

    ~FileObject() {}

    static FileObject open(const fs::path &path);

    static FileObject build_file(const fs::path &standard_path, const FILE_BASIC_INFO &attribute,
                                 const std::vector<fs::path> hard_link_refs, DataChunk &&content) {
        FileObject object;
        object.standard_path = standard_path;
        object.attribute = attribute;
        object.data.emplace<FileInfo>(hard_link_refs, std::forward<DataChunk>(content));
        return object;
    }

    static FileObject build_symlink(const fs::path &standard_path, const FILE_BASIC_INFO &attribute,
                                    const fs::path &target) {
        FileObject object;
        object.standard_path = standard_path;
        object.attribute = attribute;
        object.data.emplace<Symlink>(target);
        return object;
    }

    static FileObject build_empty_directory(const fs::path &standard_path, const FILE_BASIC_INFO &attribute) {
        FileObject object;
        object.standard_path = standard_path;
        object.attribute = attribute;
        object.data.emplace<Directory>();
        return object;
    }

    const fs::path &get_standard_path() { return standard_path; }

    void write(std::ostream &stream);
    std::string sha1();
    void recover(const fs::path &root);

private:
    struct FileInfo {
        std::vector<fs::path> hard_link_paths;
        DataChunk content;
    };

    struct Symlink {
        fs::path target;
    };

    struct Directory {};

    FILE_BASIC_INFO attribute;
    fs::path standard_path;
    std::variant<std::monostate, FileInfo, Symlink, Directory> data;

    struct FileHead {
        uint8_t magic[4];
        int object_type;
    };
};

} // namespace Syncer