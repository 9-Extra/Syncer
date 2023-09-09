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

class RepositoryAccessor {
private:
    fs::path root;
    FileList file_list;
    bool is_dirty{false};

public:
    RepositoryAccessor(const fs::path &root);

    void flush();

    void update_file(const fs::path &standard_path, const FILE_BASIC_INFO &attibute, const Syncer::DataChunk &content);

    ~RepositoryAccessor(){
        assert(!is_dirty);//必须在关闭前flush
    }
};

} // namespace Syncer