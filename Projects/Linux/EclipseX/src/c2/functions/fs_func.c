#include <linux/fs.h>

#include <linux/rculist.h> // для rcu list (если в ядре)
#include <linux/rcupdate.h> // для rcu_read_lock/unlock
#include </usr/src/linux-source-6.11/fs/mount.h>
// #include <linux/dcache.h> // struct dentry
#include <linux/path.h> // stuct path
#include <linux/namei.h> // kern_path




#include "fs_func.h"

// подразумевается, что вывод не выйдет за 1024 байт
void list_all_fs_type(char *buf, size_t buf_size) {
    struct file_system_type *fs_type;
    size_t offset = 0;

    // Получаю адрес file_system_type так get_fs_type 
    // Тянуть адрес file_systems_lock и file_systems, используя kprobe, не получается
    // /proc/filesystems - показал, что первой идет sysfs

    // spin_lock(&file_systems_lock);

    int iter = 0;
    for (fs_type = get_fs_type("sysfs"); fs_type != NULL; fs_type = fs_type->next) {
        offset += scnprintf(buf + offset, buf_size - offset, "%d|%s|%d\n", ++iter, fs_type->name, !hlist_empty(&fs_type->fs_supers));
        if (offset >= buf_size) {
            break;
        }
    }

    // spin_unlock(&file_systems_lock);
    return;
}

void list_all_sb(char *buf, size_t buf_size, char *fs_name) {
    struct file_system_type *fs_type = get_fs_type(fs_name);
    struct super_block *sb;
    struct mount *mnt;
    static char path_buf[PATH_MAX];
    char *path_str;

    size_t offset = 0;
    size_t sb_offset = 0;


    rcu_read_lock(); // Защита от гонок

    if (fs_type) {
        hlist_for_each_entry_rcu(sb, &fs_type->fs_supers, s_instances) {
            offset += scnprintf(buf + offset, buf_size - offset, "%s\n", sb->s_id);
            if (offset >= buf_size) {
                break;
            }

            list_for_each_entry(mnt, &sb->s_mounts, mnt_instance) {
                memset(path_buf, 0, PATH_MAX);
                struct path path = {
                    .mnt = &mnt->mnt,
                    .dentry = mnt->mnt.mnt_root,
                };
                path_str = d_path(&path, path_buf, sizeof(path_buf));

                if (!IS_ERR(path_str) && !strstr(buf + sb_offset, path_str)) {
                    offset += scnprintf(buf + offset, buf_size - offset, "  %s\n", path_str);
                    if (offset >= buf_size) {
                        break;
                    }
                }
            }

            sb_offset = offset;
        }
    }

    rcu_read_unlock();
    return;
}

static char path_current[PATH_MAX] = "";
static int path_current_offset = 0;
void list_dir(char *buf, size_t buf_size, char *path_str) {
    struct path path;
    struct dentry *dentry;
    struct dentry *child;
    size_t offset = 0;
    
    scnprintf(buf + path_current_offset, sizeof(buf) - path_current_offset, "/%s", path_str);
    pr_info("%s\n", path_current);
    
    if (kern_path(path_str, LOOKUP_FOLLOW, &path)) {
        offset += scnprintf(buf + offset, buf_size - offset, "Invalid path: %s\n", path_str);

        goto out;
    }

    dentry = path.dentry;
    if (S_ISDIR(dentry->d_inode->i_mode)) {
        spin_lock(&dentry->d_lockref.lock);

        hlist_for_each_entry(child, &dentry->d_children, d_sib) {
            offset += scnprintf(buf + offset, buf_size - offset, "  %s\n", child->d_name.name);
            if (offset >= buf_size) {
                break;
            }
            printk(KERN_INFO "Child name: %pd\n", child);
        }

        spin_unlock(&dentry->d_lockref.lock);
    } else {
        offset += scnprintf(buf + offset, buf_size - offset, "Not dir %s\n", path_str);

        goto out;
    }
    
out:
    memset(path_current, 0, sizeof(path_current) - path_current_offset);
    return;
}

