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
    char const *uuid;           // 如果要新建则留空("")，修改现有的则填对应的
    bool immedate_backup; // 是否立即进行一次备份
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

// 注册或者修改一个仓库，返回值表示是否出现错误，仓库的uuid返回值！会写到uuid中，字符串长度（包含结尾）40字节
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

size_t get_repository_count(); // 当前获取仓库数，注意它和用仓库句柄获取的仓库数不一定一致

struct LISTHANDLE;
// 获取仓库列表，以一个句柄的方式返回，使用此句柄迭代和访问列表（注意这相当于一个快照，如果仓库列表列表发生更新，需要重新获取句柄才能获取更新后的仓库信息）
bool list_repository_info(LISTHANDLE** handle);
// 获取列表中项目数
size_t get_repository_list_size(LISTHANDLE* handle); 
// 获取当前指向的仓库信息，index为下标，如果大于等于项目数则info内容无效，不要修改info中的内容
void get_repository_info(LISTHANDLE *handle, size_t index, RepositoryInfo *info) ;
// 关闭句柄以释放内存
void close_list_handle(LISTHANDLE* handle);

bool immedately_backup_repository(const char *uuid);
bool delete_repository(const char *uuid);

// 从备份恢复，对于无密码的仓库可以无视password
bool recover_repository(const char *uuid, const char* password);
}
} // namespace Syncer
