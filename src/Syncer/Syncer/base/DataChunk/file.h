#pragma once

#include <cstdint>
#include <filesystem>
#include "DataChunk.h"
#include "../SyncerException.h"
#include "HandleWrapper.h"

template <> struct std::hash<FILE_ID_INFO> {
    size_t operator()(FILE_ID_INFO info) const {
        union Magic {
            struct {
                uint64_t high, low;
            };
            FILE_ID_128 file_id;
        };
        static_assert(sizeof(Magic) == sizeof(info.FileId));
        Magic *magic = (Magic *)&info.FileId;
        return info.VolumeSerialNumber ^ magic->high ^ magic->low;
    }
};

template <> struct std::equal_to<FILE_ID_INFO> {
    size_t operator()(const FILE_ID_INFO &lft, const FILE_ID_INFO &rht) const {
        struct Magic {
            uint64_t data[3];
        };
        static_assert(sizeof(Magic) == sizeof(FILE_ID_INFO));
        Magic *l = (Magic *)&lft, *r = (Magic *)&rht;

        return l->data[0] == r->data[0] && l->data[1] == r->data[1] && l->data[2] == r->data[2];
    }
};

namespace Syncer {

DataChunk read_whole_file(HANDLE handle);
void write_whole_file_and_arrtibute(const std::filesystem::path &path, const FILE_BASIC_INFO &attribute,
                                         const DataChunk &content);
//打开文件HANDLE，不追踪Symlink
HandleWrapper open_file_read(const std::filesystem::path &path);
} // namespace Syncer