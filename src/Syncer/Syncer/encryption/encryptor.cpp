#include "encryptor.h"

#include <unordered_map>
#include "凯撒加密.h"
#include "无加密.h"

namespace Syncer{

static std::unordered_map<std::string, std::unique_ptr<EncryptFactory>> encryptor_table;

void EncryptFactory::init_encryptor_table(){
    if (encryptor_table.empty()){
        encryptor_table.insert_or_assign("none", std::make_unique<无加密>());
        encryptor_table.insert_or_assign("ks256", std::make_unique<凯撒加密>());
    }
}

EncryptFactory* EncryptFactory::find_encryptor(const std::string& name){
    if (auto it = encryptor_table.find(name);it != encryptor_table.end()){
        return it->second.get();
    } else {
        return nullptr;
    }
}

}