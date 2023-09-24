#include "base/DataChunk.h"
#include "file.h"
#include "base/HandleWrapper.h"
#include "base/SyncerException.h"
#include <proto/Serialize.pb.h>
#include <fstream>
#include <iostream>

namespace Syncer {

namespace fs = std::filesystem;

FILE_ID_INFO get_file_identity(HANDLE file);

class FileFiliter {
private:
public:
    virtual bool filiter_path(const fs::path &path) const { return true; }
    virtual bool filiter_attribute(const fs::path &path, const FILE_BASIC_INFO &attribute) const { return true; }
};

void copy(const fs::path &root, const fs::path &target, const FileFiliter &filiter);

void store(const fs::path &root, const fs::path &target, const FileFiliter &filiter);
void recover(const fs::path &storage_path, const fs::path &target);

} // namespace Syncer