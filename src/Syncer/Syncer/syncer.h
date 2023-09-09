#include "config.h"

namespace Syncer {

namespace fs = std::filesystem;



RepositoryConfig load_config(const fs::path &path);

}