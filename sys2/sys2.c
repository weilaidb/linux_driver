#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/pid.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/kallsyms.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/if_vlan.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

// 创建一个 kobject 来表示 /sys/kernel/mymodule
static struct kobject *mymodule_kobj;

// 定义统计变量
static unsigned long vlan_stats[4094] = {0}; // VLAN1 到 VLAN4094 的数据包统计
static unsigned long ip_packets = 0;         // IP 数据包总数
static unsigned long tcp_packets = 0;        // TCP 数据包总数
static unsigned long udp_packets = 0;        // UDP 数据包总数
static unsigned long icmp_packets = 0;       // ICMP 数据包总数
static unsigned long other_packets = 0;      // 其他未知协议类型的数据包总数



// 用于存储用户写入的内容
static char my_data[256] = {0};

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
            pr_info("TCP packet: Source IP: %pI4, Destination IP: %pI4, Source Port: %d, Destination Port: %d\n",
                    &ip_header->saddr, &ip_header->daddr,
                    ntohs(((struct tcphdr *)(skb->data + sizeof(struct iphdr)))->source),
                    ntohs(((struct tcphdr *)(skb->data + sizeof(struct iphdr)))->dest));
            break;
        case IPPROTO_UDP:
            udp_packets++;
            pr_info("UDP packet: Source IP: %pI4, Destination IP: %pI4, Source Port: %d, Destination Port: %d\n",
                    &ip_header->saddr, &ip_header->daddr,
                    ntohs(((struct udphdr *)(skb->data + sizeof(struct iphdr)))->source),
                    ntohs(((struct udphdr *)(skb->data + sizeof(struct iphdr)))->dest));
            break;
        case IPPROTO_ICMP:
            icmp_packets++;
            pr_info("ICMP packet: Source IP: %pI4, Destination IP: %pI4, Type: %d, Code: %d\n",
                    &ip_header->saddr, &ip_header->daddr,
                    ((struct icmphdr *)(skb->data + sizeof(struct iphdr)))->type,
                    ((struct icmphdr *)(skb->data + sizeof(struct iphdr)))->code);
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


// sysfs 属性的显示函数
static ssize_t my_data_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", my_data);
}

void showtasklist(const char *param)
{
    struct task_struct *task;
    int iTaskCount = 0;

    for_each_process(task)
    {
        printk(KERN_INFO "Process: %s (PID: %d)\n", task->comm, task->pid);
        iTaskCount++;
    }
    printk(KERN_INFO "task count:%d\n", iTaskCount);
}

void print_task_info(const char *param)
{
    struct task_struct *task;
    int iTaskCount = 0;

    for_each_process(task)
    {
        pid_t pid = task->pid;
        pid_t tgid = task->tgid;
        int prio = task->prio;
        int static_prio = task->static_prio;
        int normal_prio = task->normal_prio;

        printk(KERN_INFO "Process: %s (PID: %d, TGID: %d, Prio: %d, Static Prio: %d, Normal Prio: %d)\n",
               task->comm, pid, tgid, prio, static_prio, normal_prio);
        iTaskCount++;
    }
    printk(KERN_INFO "task count:%d\n", iTaskCount);
}

// 解析用户输入的命令和参数
static void parse_command(const char *cmd)
{
    char *token, *cmd_copy;
    int param_count = 0;

    // 复制命令字符串，因为 strsep 会修改原字符串
    cmd_copy = kstrdup(cmd, GFP_KERNEL);
    if (!cmd_copy) {
        printk(KERN_ERR "Memory allocation failed\n");
        return;
    }

    // 使用 strsep 分割命令和参数
    token = strsep(&cmd_copy, " ");
    while (token) {
        param_count++;
        printk(KERN_INFO "Parameter %d: %s\n", param_count, token);
        token = strsep(&cmd_copy, " ");
    }

    kfree(cmd_copy);
}

// 设置所有进程的优先级
static void set_all_task_nice(const char *param)
{
    int nice_value = 0;
    if ((sscanf(param, "%d", &nice_value) >= 1) && (nice_value >= -20 && nice_value <= 19)) {
        struct task_struct *task;
        for_each_process(task) {
            if (task->pid != 0) { // 跳过 init 进程
                set_user_nice(task, nice_value);
                printk(KERN_INFO "Set nice value of process %s (PID: %d) to %d\n", task->comm, task->pid, nice_value);
            }
        }
    } else {
        printk(KERN_ERR "Invalid nice value\n");
    }
}

// 设置指定用户的进程的优先级
static void set_user_nice_by_uid(const char *param)
{
    int uid, nice_value;
    bool flag = false;

    if (sscanf(param, "%d %d", &uid, &nice_value) == 2) {
        struct task_struct *task;
        kuid_t kuid = make_kuid(current_user_ns(), uid);

        if (!uid_valid(kuid)) {
            printk(KERN_ERR "Invalid UID: %d\n", uid);
            return;
        }

        if (nice_value < -20 || nice_value > 19) {
            printk(KERN_ERR "Invalid nice value: %d\n", nice_value);
            return;
        }

        for_each_process(task) {
            if (task->pid != 0 && uid_eq(task_uid(task), kuid)) { // 跳过 init 进程
                set_user_nice(task, nice_value);
                printk(KERN_INFO "Set nice value of process %s (PID: %d, UID: %d) to %d\n", task->comm, task->pid, uid, nice_value);
                flag = true;
            }
        }
        if (!flag) {
            printk(KERN_ERR "No process found with UID: %d\n", uid);
        }
    } else {
        printk(KERN_ERR "Invalid input format. Expected: setniceuser <uid> <nice_value>\n");
    }
}

// 打印内核符号表中的所有符号
static void showkernelsymbol(const char *param)
{
    // unsigned long addr;
    // const char *symbol;

    // printk(KERN_INFO "Kernel symbol table:\n");
    // for (addr = 0; addr < ULONG_MAX; addr += 4) {
    //     symbol = kallsyms_lookup(addr, NULL, NULL, NULL, NULL);
    //     if (symbol) {
    //         printk(KERN_INFO "[%016lx] %s\n", addr, symbol);
    //     }
    // }
}

// 定义命令处理函数的结构体
struct command_handler {
    const char *cmd;
    void (*handler)(const char *param);
};

// 定义命令处理函数数组
static struct command_handler command_handlers[] = {
    {"showtask", showtasklist},
    {"showalltask", print_task_info},
    {"parse", parse_command},
    {"setniceuser", set_user_nice_by_uid},
    {"setnice", set_all_task_nice},
    {"showkernelsymbol", showkernelsymbol},
    {NULL, NULL} // 结束标志
};

// sysfs 属性的存储函数
static ssize_t my_data_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int i;
    const char *param;

    snprintf(my_data, sizeof(my_data), "%s", buf);
    printk(KERN_INFO "Received data: %s\n", my_data);

    // 遍历命令处理函数数组
    for (i = 0; command_handlers[i].cmd; i++) {
        if (strncmp(my_data, command_handlers[i].cmd, strlen(command_handlers[i].cmd)) == 0) {
            param = my_data + strlen(command_handlers[i].cmd) + 1;
            command_handlers[i].handler(param);
            return count;
        }
    }

    printk(KERN_ERR "Unknown command: %s\n", my_data);
    return count;
}

// 定义 sysfs 属性
static struct kobj_attribute my_data_attr = __ATTR(my_data, 0644, my_data_show, my_data_store);

// 模块初始化函数
static int __init mymodule_init(void)
{
    int ret;

    mymodule_kobj = kobject_create_and_add("mymodule", kernel_kobj);
    if (!mymodule_kobj)
    {
        printk(KERN_ERR "Failed to create kobject\n");
        return -ENOMEM;
    }

    ret = sysfs_create_file(mymodule_kobj, &my_data_attr.attr);
    if (ret)
    {
        kobject_put(mymodule_kobj);
        printk(KERN_ERR "Failed to create sysfs file\n");
        return ret;
    }

    ret = register_net_hooks();
    if (ret)
    {
        sysfs_remove_file(mymodule_kobj, &my_data_attr.attr);
        kobject_put(mymodule_kobj);
        printk(KERN_ERR "Failed to register net hooks\n");
        return ret;
    }

    printk(KERN_INFO "My module loaded.\n");
    return 0;
}

static void __exit mymodule_exit(void)
{
    unregister_net_hooks();
    sysfs_remove_file(mymodule_kobj, &my_data_attr.attr);
    kobject_put(mymodule_kobj);
    printk(KERN_INFO "My module unloaded.\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple module to create a sysfs entry under /sys/kernel");
MODULE_AUTHOR("Your Name");