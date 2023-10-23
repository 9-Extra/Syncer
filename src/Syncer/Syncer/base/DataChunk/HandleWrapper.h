#pragma once

#include "../winapi.h"
#include <filesystem>

namespace Syncer {
struct HandleWrapper {
    HANDLE handle;

    HandleWrapper(HANDLE handle = INVALID_HANDLE_VALUE) : handle(handle){}

    HandleWrapper(HandleWrapper&& other) : handle(other.handle){
        other.handle = INVALID_HANDLE_VALUE;
    }

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
}
