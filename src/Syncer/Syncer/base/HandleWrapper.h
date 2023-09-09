#pragma once

#include "winapi.h"
#include <filesystem>

namespace Syncer {
struct HandleWrapper {
    HANDLE handle;

    HandleWrapper(HANDLE handle = INVALID_HANDLE_VALUE) : handle(handle){}

    bool is_vaild() { return handle != INVALID_HANDLE_VALUE; }

    HandleWrapper& operator=(HandleWrapper&& other){
        this->handle = other.handle;
        other.handle = INVALID_HANDLE_VALUE;
        return *this;
    }
    HandleWrapper& operator=(const HandleWrapper& other) = delete;

    ~HandleWrapper() {
        if (is_vaild()) {
            CloseHandle(handle);
        }
    }
};

//打开文件HANDLE，不追踪Symlink
inline HandleWrapper open_file_read(const std::filesystem::path &path) {
    return HandleWrapper{
        CreateFileW(path.wstring().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OPEN_REPARSE_POINT, NULL)};
}
}
