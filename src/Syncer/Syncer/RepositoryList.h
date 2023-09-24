#pragma once

#include <vector>
#include "Repository/config.h"

namespace Syncer {

struct RepositoryList{
    std::vector<RepositoryConfig> resp_list;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RepositoryList, resp_list);
};

extern fs::path json_file_path;
extern RepositoryList repository_list;

void init_json_file_path();
void load_config_file();
void save_config_file();
}