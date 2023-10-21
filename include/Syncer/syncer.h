#pragma once

#include <stdbool.h>
#include <stdint.h>


namespace Syncer {

extern "C" {

bool init_backup_system();
bool stop_backup_system();

// 获取错误信息，如果没有错误返回false，否则返回true且将错误信息填入reason，buffer_size指定reason大小防止越界，一般错误字符串不超过128个字节
// 获取错误后错误信息将清空，如果发生多个错误，新的会将旧的覆盖
// 本函数不会出错
bool get_error_reason(char *reason, size_t buffer_size); 

struct AutoBackupDesc {
    unsigned int interval;
};
struct RepositoryDesc {
    char const *custom_name; // 用户自定义名字，可以留空
    char const *source_path; // 要备份的文件夹路径
    char const *target_path; // 存储的路径，必须是一个文件夹路径（不存在会自动创建），而对于打包必须是一个文件路径
    char const *filter;         // 过滤器，未实现

    char const *encrypt_method; // 加密算法，可选none（无加密），ks256（凯撒加密）
    char const *password;     // 密码，无加密则填空字符串""

    bool do_packup; // 以打包的形式备份
    bool enable_autobackup;
    AutoBackupDesc auto_backup_config;
};

// 注册一个仓库，返回值表示是否出现错误，仓库的uuid返回值会写到uuid中，uuid字符串长度需要（包含结尾）40字节
// 如果要“修改”一个仓库的配置，就获取现有的那个仓库的信息，然后删掉新建！
// 注册过程中会立即进行一次备份
bool register_repository(const RepositoryDesc *desc, char* uuid);

struct RepositoryInfo {
    const char *uuid;
    const char *custom_name;
    const char *source_path;
    const char *target_path;
    const char *filter;
    const char *last_backup_time;

    uint32_t file_number;
    bool need_password;

    bool packup;
    bool enable_autobackup;
    AutoBackupDesc auto_backup_config;
};

struct LISTHANDLE;
// 获取仓库列表，以一个句柄的方式返回，使用此句柄迭代和访问列表（注意这相当于一个快照，如果仓库列表列表发生更新，需要重新获取句柄才能获取更新后的仓库信息）
bool list_repository_info(LISTHANDLE** handle);
// 获取一个指定仓库的信息，返回的方法和上一个函数一样，只是列表里只有1个数据（uuid不存在则列表无效且返回false）
bool list_repository_info_uuid(LISTHANDLE** handle, const char* uuid);
// 获取列表中项目数
size_t get_repository_list_size(LISTHANDLE* handle); 
// 获取当前指向的仓库信息，index为下标，如果大于等于项目数则info内容无效，不要修改info中的内容
void get_repository_info(LISTHANDLE *handle, size_t index, RepositoryInfo *info) ;
// 关闭句柄以释放内存，只有获取句柄的函数返回true时才需要执行此函数
void close_list_handle(LISTHANDLE* handle);
// 删除一个仓库，注意删除仓库时是不会进行恢复操作的，并且会删除备份数据
bool delete_repository(const char *uuid);

// 立即对一个仓库进行备份操作
bool immedately_backup_repository(const char *uuid);
// 从备份恢复，对于无密码的仓库可以无视password
bool recover_repository(const char *uuid, const char* password);
}
} // namespace Syncer
