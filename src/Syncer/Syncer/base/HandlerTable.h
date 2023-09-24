#pragma once

#include <vector>
#include <unordered_map>
#include "SyncerException.h"

namespace Syncer {
template <class T>

class HandlerTable{
public:
    struct IDData{
        T inner;
        uint32_t id;
    };

    uint32_t alloc(){
        uint32_t id = ++id_alloc;
        for(;id_index_map.contains(id);id = ++id_alloc);
        
    }

    void remove(uint32_t id){
        if (!id_index_map.contains(id)){
            throw SyncerException("id不存在");
        }

        size_t index = id_index_map.at(id);
        uint32_t id_back = data.back().id;

        std::swap(data[index], data.back());
        data.pop_back();

        id_index_map.erase(id);
        id_index_map[id_back] = index;
    }

    T& get(uint32_t id){
        return data[id].inner;
    }

    const T& get(uint32_t id) const{
        return data[id].inner;
    }

    const std::vector<IDData>& list() const{
        return data;
    }
private:
    std::vector<IDData> data;
    std::unordered_map<uint32_t, size_t> id_index_map;
    uint32_t id_alloc = 0;

};

}