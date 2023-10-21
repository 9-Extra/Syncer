#include "DataChunk.h"
#include "winapi.h"
#include "SyncerException.h"
#include <format>
namespace Syncer {
void DataChunk::write_to_file(std::filesystem::path file_path, bool override) {
    HANDLE file = CreateFileW(file_path.wstring().c_str(), GENERIC_WRITE, 0, NULL,
                              override ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        throw SyncerException(std::format("无法创建文件. Error id: {}" , GetLastError()));
    }

    DWORD written = 0;
    uint8_t *current = (uint8_t *)this->start;
    uint8_t *end = current + this->size;
    while (current < end) {
        DWORD to_write = (DWORD)std::min<uint64_t>(end - current, std::numeric_limits<DWORD>::max());
        if (!WriteFile(file, current, to_write, &written, nullptr)) {
            CloseHandle(file);
            throw SyncerException(std::format("写文件时出错. Error id: {}" , GetLastError()));
        };
        current += written;
    }

    CloseHandle(file);
}
} // namespace Syncer