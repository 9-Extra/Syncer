#pragma once

#include <filesystem>
#include <span>
#include <vector>

namespace Syncer {
template <class T> std::span<const uint8_t> fill_struct(T *stu, std::span<const uint8_t> data) {
    assert(sizeof(T) <= data.size_bytes());
    memcpy(stu, data.data(), sizeof(T));
    return data.subspan(sizeof(T));
}
struct DataChunk {
    void *start;
    uint64_t size;

    DataChunk() : start(nullptr), size(0) {}
    DataChunk(uint64_t size) {
        start = malloc(size);
        this->size = size;
    }

    DataChunk(const DataChunk &other) = delete;

    DataChunk(DataChunk &&other) : start(other.start), size(other.size) {
        other.start = nullptr;
        other.size = 0;
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