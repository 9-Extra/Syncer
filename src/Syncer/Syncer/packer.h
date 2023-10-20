#include "encryption/encryptor.h"

namespace Syncer {

namespace fs = std::filesystem;

void copy(const fs::path &root, const fs::path &target, const std::string &filiter);

void store(const fs::path &root, const fs::path &target, const std::string &filiter, EncryptFactory::Encoder* encoder);
void recover(const fs::path &storage_path, const fs::path &target, EncryptFactory::Decoder* decoder);

void pack(const fs::path &root, const fs::path &target, const std::string &filiter);
void unpack(const fs::path &pack_file, const fs::path &target);

} // namespace Syncer