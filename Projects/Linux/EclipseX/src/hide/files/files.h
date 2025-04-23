#ifndef FILES_H
#define FILES_H

#include "../../hook_functions.h"

bool is_hide_uid(unsigned int file_uid);
bool is_hide_gid(unsigned int file_gid);
bool copy_filepath(char *filepath, char **kfilepath);
bool is_hide_file(unsigned int file_uid, unsigned int file_gid, char *kfilepath);

#endif

