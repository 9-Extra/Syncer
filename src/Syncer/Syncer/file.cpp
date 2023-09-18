#include "file.h"

namespace Syncer {

namespace fs = std::filesystem;

DataChunk read_whole_file(HANDLE handle) {
    LARGE_INTEGER size;
    if (!GetFileSizeEx(handle, &size)) {
        throw SyncerException(std::format("获取文件大小失败: ", GetLastError()));
    }
    DataChunk chunk(size.QuadPart);

    char *ptr = (char *)chunk.start;
    char *end = ptr + chunk.size;
    DWORD read = 0;
    while (ptr < end) {
        DWORD to_read = (DWORD)std::min<size_t>(std::numeric_limits<DWORD>::max(), end - ptr);
        if (!ReadFile(handle, ptr, to_read, &read, NULL)) {
            throw SyncerException(std::format("读取文件失败: ", GetLastError()));
        }
        ptr += read;
    }

    return chunk;
}

void write_whole_file_and_arrtibute(const fs::path &path, const FILE_BASIC_INFO &attribute, const DataChunk &content) {
    fs::path dir(path);
    fs::create_directories(dir.remove_filename());
    HANDLE handle =
        CreateFileW(path.wstring().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    char *ptr = (char *)content.start;
    char *end = ptr + content.size;
    DWORD written = 0;
    while (ptr < end) {
        DWORD to_write = (DWORD)std::min<size_t>(std::numeric_limits<DWORD>::max(), end - ptr);
        if (!WriteFile(handle, ptr, to_write, &written, NULL)) {
            throw SyncerException(std::format("写入文件失败: ", GetLastError()));
        }
        ptr += written;
    }

    if (!SetFileInformationByHandle(handle, FileBasicInfo, (void *)&attribute, sizeof(FILE_BASIC_INFO))) {
        throw SyncerException("设置文件属性失败");
    }

    CloseHandle(handle);
}
HandleWrapper open_file_read(const std::filesystem::path &path) {
    HANDLE handle =
        CreateFileW(path.wstring().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS , NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        throw SyncerException(std::format("打开文件 {} 失败", path.string()));
    }

    return HandleWrapper(handle);
}
} // namespace Syncer