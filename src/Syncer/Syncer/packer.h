#include "base/DataChunk.h"
#include "config.h"
#include "file.h"
#include "base/HandleWrapper.h"
#include "base/SyncerException.h"
#include <Serialize.pb.h>
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

void pack(const fs::path &root, const fs::path &target, const FileFiliter &filiter);

} // namespace Syncer