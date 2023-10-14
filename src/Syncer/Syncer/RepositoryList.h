#pragma once

#include <vector>
#include "Repository/config.h"

namespace Syncer {

struct RepositoryList{
    std::unordered_map<std::string, RepositoryConfig> resp_list;
    
    friend void to_json(nlohmann::json& j, const RepositoryList& t) { 
        j = nlohmann::json::array();
        for(const auto& [uuid, c] : t.resp_list){
            j.emplace_back(c);
        }
    }
    friend void from_json(const nlohmann::json& j, RepositoryList& t) { 
        for(auto&& it : j){
            t.resp_list.emplace(it.at("uuid").get<std::string>(), it.get<RepositoryConfig>());
        }
    }

};

extern fs::path json_file_path;
extern RepositoryList repository_list;

void init_json_file_path();
void load_config_file();
void save_config_file();
}