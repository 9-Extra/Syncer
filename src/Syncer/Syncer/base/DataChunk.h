#pragma once

#include <filesystem>
#include "sha1.hpp"
#include <assert.h>

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

    DataChunk(const DataChunk& other) = delete;

    DataChunk clone() const{
        DataChunk d(size);
        memcpy(d.start, this->start, this->size);
        return d;
    }

    // DataChunk(const DataChunk& other) : DataChunk(other.size){
    //     memcpy(this->start, other.start, this->size);
    //     std::cout << "DataChunk Copy!!!" << std::endl;
    // }

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

    SHAResult sha1() const{
        SHA1 sha;
        sha.update((char*)start, size);
        return sha.final();
    }

    template <class T> static DataChunk store(T *data){
        DataChunk chunk(sizeof(T));
        memcpy(chunk.start, (void *)data, sizeof(T));
        return chunk;
    }

    ~DataChunk() {
        if (start != nullptr) {
            free(start);
        }
    }

    void write_to_file(std::filesystem::path file_path, bool override = true);
};

struct DataSpan{
    char *start;
    uint64_t size;

    static DataSpan from_chunk(DataChunk& chunk){
        return DataSpan((char*)chunk.start, chunk.size);
    }

    const static DataSpan from_chunk(const DataChunk& chunk){
        return DataSpan((char*)chunk.start, chunk.size);
    }
    SHAResult sha1(){
        SHA1 sha;
        sha.update(start, size);
        return sha.final();
    }

};

struct DataChunkWriter{
    char* ptr;
    char* end;

    DataChunkWriter(DataChunk& chunk){
        ptr = (char*)chunk.start;
        end = ptr + chunk.size;
    }

    DataChunkWriter(DataSpan& chunk){
        ptr = (char*)chunk.start;
        end = ptr + chunk.size;
    }

    void write_buf(const void* buf, size_t size){
        assert(ptr + size <= end);
        memcpy(ptr, buf, size);
        ptr += size;
    }

    template<class T>
    void write(const T& item){
        write_buf(&item, sizeof(T));
    }
};

} // namespace Syncer