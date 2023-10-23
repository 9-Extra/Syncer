#include "uuid.h"
#include "../winapi.h"
#include "../SyncerException.h"

namespace Syncer {
    std::string generate_guid_string(){
        UUID uuid;
        if (UuidCreate(&uuid)  != RPC_S_OK){
            throw SyncerException("生成UUID失败");
        }
        RPC_CSTR str;
        if (UuidToStringA(&uuid, &str) != RPC_S_OK){
            throw SyncerException("生成UUID失败");
        }
        std::string result((char*)str, 36);

        RpcStringFree(&str);

        return result;
    }

    UUID generate_uuid() {
        UUID uuid;
        if (UuidCreate(&uuid) != RPC_S_OK) {
            throw SyncerException("生成UUID失败");
        };
        return uuid;
    }
    } // namespace Syncer