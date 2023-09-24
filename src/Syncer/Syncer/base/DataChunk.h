#pragma once

#include <filesystem>
#include <vector>
#include <iostream>

namespace Syncer {
template <class T> void* fill_struct(T &stu, const void* ptr) {
    memcpy(&stu, ptr, sizeof(T));
    return (void*)((char*)ptr + sizeof(T));
}
struct DataChunk {
    void *start;
    uint64_t size;

    DataChunk() : start(nullptr), size(0) {}
    DataChunk(uint64_t size) {
        start = malloc(size);
        this->size = size;
    }

    DataChunk(const DataChunk& other) : DataChunk(other.size){
        memcpy(this->start, other.start, this->size);
        std::cout << "DataChunk Copy!!!" << std::endl;
    }

    DataChunk(DataChunk &&other) : start(other.start), size(other.size) {
        other.start = nullptr;
        other.size = 0;
        std::cout << "DataChunk move" << std::endl;
    }

    DataChunk &operator=(DataChunk &&other) {
        if (this != &other) {
            start = other.start;
            size = other.size;
            other.start = nullptr;
            other.size = 0;
        }
        return *this;
    }

    template <class T> static DataChunk store(T *data) {
        DataChunk chunk(sizeof(T));
        memcpy(chunk.start, (void *)data, sizeof(T));
        return chunk;
    }

    ~DataChunk() {
        if (start != nullptr) {
            free(start);
        }
    }

    bool write_to_file(std::filesystem::path file_path, bool override = false);
};
} // namespace Syncer