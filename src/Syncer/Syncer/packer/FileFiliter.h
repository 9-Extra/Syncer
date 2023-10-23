#pragma once

#include "../base/glob/glob.h"
#include "../base/winapi.h"


#include <filesystem>
#include <iostream>
#include <ranges>
#include <string>


namespace Syncer {

namespace fs = std::filesystem;

class FileFiliter {
public:
    FileFiliter(const std::string &desc) {
        auto split = desc | std::ranges::views::split('\n') | std::ranges::views::transform([](auto &&str) {
                         return std::string(&*str.begin(), std::ranges::distance(str));
                     });

        for (auto &&word : split) {
            regexs.push_back(glob::compile_pattern(word));
        }
    }

    bool filiter_path(const fs::path &path) const { 
        for(const std::regex& r : regexs){
            if (std::regex_match(path.string(), r)){
                return false;
            }    
        }
        return true;
    }
    bool filiter_attribute(const fs::path &path, const FILE_BASIC_INFO &attribute) const { return filiter_path(path); }

private:
    std::vector<std::regex> regexs;
};
} // namespace Syncer