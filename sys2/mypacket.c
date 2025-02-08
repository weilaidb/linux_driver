/***************************************************************************
** 版权所有:   Copyright (c) 2020-2030
** 文件名称:  mypacket.c
** 当前版本:  v1.0
** 作    者:
** 完成日期: 2025-02-08
** 修改记录:
** 修改日期:
** 版本号  :
** 修改人  :
** 修改内容:
***************************************************************************/

/*****************************头文件****************************************/

/*****************************宏定义****************************************/

/*****************************结构体或类型定义*******************************/

/*****************************全局变量****************************************/

/*****************************本地变量****************************************/
// 定义统计变量
static unsigned long vlan_stats[4094] = {0}; // VLAN1 到 VLAN4094 的数据包统计
static unsigned long ip_packets = 0;         // IP 数据包总数
static unsigned long tcp_packets = 0;        // TCP 数据包总数
static unsigned long udp_packets = 0;        // UDP 数据包总数
static unsigned long icmp_packets = 0;       // ICMP 数据包总数
static unsigned long other_packets = 0;      // 其他未知协议类型的数据包总数

/*****************************函数或类声明****************************************/

/*****************************函数或类实现****************************************/

// 网络钩子函数
unsigned int packet_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct vlan_hdr *vlan_hdr;
    struct iphdr *ip_header;
    unsigned int vlan_id = 0;

    // 检查是否是 VLAN 数据包
    if (skb->protocol == htons(ETH_P_8021Q))
    {
        vlan_hdr = (struct vlan_hdr *)skb->data;
        vlan_id = ntohs(vlan_hdr->h_vlan_TCI) & VLAN_VID_MASK;
        if (vlan_id >= 1 && vlan_id <= 4094)
        {
            vlan_stats[vlan_id - 1]++; // VLAN 数据包统计
        }
        skb_pull(skb, sizeof(struct vlan_hdr)); // 跳过 VLAN 头部
        skb_reset_network_header(skb);          // 确保 data 指向 IP 头部
    }

    // 检查是否是 IP 数据包
    if (skb->protocol == htons(ETH_P_IP) || skb->protocol == htons(ETH_P_IPV6))
    {
        ip_header = ip_hdr(skb); // 使用宏获取 IP 头部
        ip_packets++;            // IP 数据包统计

        // 根据协议类型统计 TCP、UDP 和 ICMP 数据包
        switch (ip_header->protocol)
        {
        case IPPROTO_TCP:
            tcp_packets++;
            // pr_info("TCP packet: Source IP: %pI4, Destination IP: %pI4, Source Port: %d, Destination Port: %d\n",
            //         &ip_header->saddr, &ip_header->daddr,
            //         ntohs(((struct tcphdr *)(skb->data + sizeof(struct iphdr)))->source),
            //         ntohs(((struct tcphdr *)(skb->data + sizeof(struct iphdr)))->dest));
            break;
        case IPPROTO_UDP:
            udp_packets++;
            // pr_info("UDP packet: Source IP: %pI4, Destination IP: %pI4, Source Port: %d, Destination Port: %d\n",
            //         &ip_header->saddr, &ip_header->daddr,
            //         ntohs(((struct udphdr *)(skb->data + sizeof(struct iphdr)))->source),
            //         ntohs(((struct udphdr *)(skb->data + sizeof(struct iphdr)))->dest));
            break;
        case IPPROTO_ICMP:
            icmp_packets++;
            // pr_info("ICMP packet: Source IP: %pI4, Destination IP: %pI4, Type: %d, Code: %d\n",
            //         &ip_header->saddr, &ip_header->daddr,
            //         ((struct icmphdr *)(skb->data + sizeof(struct iphdr)))->type,
            //         ((struct icmphdr *)(skb->data + sizeof(struct iphdr)))->code);
            break;
        default:
            other_packets++; // 其他未知协议类型的数据包统计
            break;
        }
    }
    else
    {
        other_packets++; // 其他未知协议类型的数据包统计
    }

    return NF_ACCEPT; // 允许数据包继续传输
}

// 定义全局的 nf_hook_ops 结构体
static struct nf_hook_ops nf_ops = {
    .hook = packet_hook,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST,
};

// 注册网络钩子
static int register_net_hooks(void)
{
    nf_register_net_hook(&init_net, &nf_ops);
    return 0;
}

// 注销网络钩子
static void unregister_net_hooks(void)
{
    nf_unregister_net_hook(&init_net, &nf_ops);
    nf_ops.hook = NULL; // 注销后将钩子函数指针设置为 NULL
}
