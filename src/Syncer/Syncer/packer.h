#include "base/DataChunk.h"
#include "file.h"

namespace Syncer {

namespace fs = std::filesystem;

FILE_ID_INFO get_file_identity(HANDLE file);

void copy(const fs::path &root, const fs::path &target, const std::string &filiter);

void store(const fs::path &root, const fs::path &target, const std::string &filiter);
void recover(const fs::path &storage_path, const fs::path &target);

void pack(const fs::path &root, const fs::path &target, const std::string &filiter);
void unpack(const fs::path &pack_file, const fs::path &target);

} // namespace Syncer