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

#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sched/prio.h>
#include <linux/pid.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <net/rtnetlink.h>
#include <net/net_namespace.h>
#include <linux/inet.h>     // 用于处理 IP 地址
#include <linux/in.h>       // 用于处理 IPv4 地址
#include <linux/if_ether.h> // 用于处理以太网相关字段
// #include <net/ipv4.h>       // 用于处理 IPv4 地址
// #include <net/inetdev.h>    // 用于处理网络设备的 IPv4 地址


// 显示网络接口统计信息的函数
// 去除字符串末尾的空格和换行符
static void trim_trailing_whitespace(char *str)
{
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
    {
        *end = '\0';
        end--;
    }
}

// 显示网络接口统计信息的函数
int showinterface_in(const char *param)
{
    struct net_device *dev = NULL;
    struct rtnl_link_stats64 stats;
    char interface_name[IFNAMSIZ];
    struct in_device *in_dev;
    struct in_ifaddr *ifa;

    // 检查输入是否为空
    if (!param || strlen(param) == 0)
    {
        printk(KERN_INFO "Invalid input format. Expected: showinterface <interface_name>\n");
        return -EINVAL;
    }

    // 复制接口名称并去除末尾的空格和换行符
    strncpy(interface_name, param, IFNAMSIZ - 1);
    interface_name[IFNAMSIZ - 1] = '\0';
    trim_trailing_whitespace(interface_name);

    printk(KERN_INFO "Showing interface statistics: %s\n", interface_name);

    // 查找指定的网络接口
    dev = dev_get_by_name(&init_net, interface_name);
    if (!dev)
    {
        printk(KERN_INFO "Interface %s not found\n", interface_name);
        return -ENODEV;
    }

    // 获取网络接口的统计信息
    dev_get_stats(dev, &stats);

    // 打印统计信息
    printk(KERN_INFO "Interface: %s\n", dev->name);
    printk(KERN_INFO "MTU: %d\n", dev->mtu);
    printk(KERN_INFO "MAC Address: %pM\n", dev->dev_addr);
    printk(KERN_INFO "RX packets: %llu\n", stats.rx_packets);
    printk(KERN_INFO "RX bytes: %llu\n", stats.rx_bytes);
    printk(KERN_INFO "RX errors: %llu\n", stats.rx_errors);
    printk(KERN_INFO "RX dropped: %llu\n", stats.rx_dropped);
    printk(KERN_INFO "TX packets: %llu\n", stats.tx_packets);
    printk(KERN_INFO "TX bytes: %llu\n", stats.tx_bytes);
    printk(KERN_INFO "TX errors: %llu\n", stats.tx_errors);
    printk(KERN_INFO "TX dropped: %llu\n", stats.tx_dropped);

    // // 打印 IPv4 地址
    // rtnl_lock();
    // in_dev = __in_dev_get_rtnl(dev);
    // if (in_dev)
    // {
    //     printk(KERN_INFO "IPv4 Addresses:\n");
    //     for (ifa = in_dev->ifa_list; ifa; ifa = ifa->ifa_next)
    //     {
    //         printk(KERN_INFO "  %pI4/%d\n", &ifa->ifa_address, ifa->ifa_prefixlen);
    //     }
    // }
    // rtnl_unlock();

    dev_put(dev); // 释放网络设备引用
    return 0;
}

void showinterface(const char *param)
{
    showinterface_in(param);
}

// 设置网络接口的 MTU 值
int setmtu_in(const char *param)
{
    struct net_device *dev = NULL;
    char interface_name[IFNAMSIZ];
    int new_mtu;

    // 检查输入是否为空
    if (!param || strlen(param) == 0)
    {
        printk(KERN_INFO "Invalid input format. Expected: setmtu <interface_name> <mtu_value>\n");
        return -EINVAL;
    }

    // 解析参数
    if (sscanf(param, "%s %d", interface_name, &new_mtu) != 2)
    {
        printk(KERN_INFO "Invalid input format. Expected: setmtu <interface_name> <mtu_value>\n");
        return -EINVAL;
    }

    // 去除接口名称末尾的空格和换行符
    trim_trailing_whitespace(interface_name);

    printk(KERN_INFO "Setting MTU for interface: %s to %d\n", interface_name, new_mtu);

    // 查找指定的网络接口
    dev = dev_get_by_name(&init_net, interface_name);
    if (!dev)
    {
        printk(KERN_INFO "Interface %s not found\n", interface_name);
        return -ENODEV;
    }

    // 设置新的 MTU 值
    if (new_mtu < 68 || new_mtu > 65535) // MTU 值范围
    {
        printk(KERN_INFO "Invalid MTU value: %d. Valid range is 68 to 65535\n", new_mtu);
        dev_put(dev);
        return -EINVAL;
    }

    dev->mtu = new_mtu;
    printk(KERN_INFO "MTU for interface %s set to %d\n", dev->name, dev->mtu);

    dev_put(dev); // 释放网络设备引用
    return 0;
}


void setmtu(const char *param)
{
    setmtu_in(param);
}