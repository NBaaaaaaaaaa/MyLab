#ifndef FS_FUNC_H
#define FS_FUNC_H

void list_all_fs_type(char *buffer, size_t buffer_size);
void list_all_sb(char *buf, size_t buf_size, char *fs_name);
void list_dir(char *buf, size_t buf_size, char *path_str);
#endif
