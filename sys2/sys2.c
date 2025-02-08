/***************************************************************************
** 版权所有:   Copyright (c) 2020-2030
** 文件名称:  sys2.c
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
#include <linux/sched/signal.h>
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



//引用头文件不行，先引用源文件
#include "showtask.c"
#include "parse.c"
#include "mypacket.c"


/*****************************宏定义****************************************/

/*****************************结构体或类型定义*******************************/

// 定义命令处理函数的结构体
struct command_handler
{
    const char *cmd;
    void (*handler)(const char *param);
};

/*****************************全局变量****************************************/

/*****************************本地变量****************************************/

// 创建一个 kobject 来表示 /sys/kernel/mymodule
static struct kobject *mymodule_kobj;

// 用于存储用户写入的内容
static char my_data[256] = {0};

// 定义命令处理函数数组
static struct command_handler command_handlers[] = {
    {"showtask", showtasklist},
    {"showalltask", print_task_info},
    {"parse", parse_command},
    {"setniceuser", set_user_nice_by_uid},
    {"setnice", set_all_task_nice},
    {"showkernelsymbol", showkernelsymbol},
    {"showallthread", showallthread},           // 添加 showallthread 命令
    {"setthreadpriority", set_thread_priority}, // 修改线程优先级命令
    {NULL, NULL}                                // 结束标志
};

/*****************************函数或类声明****************************************/

/*****************************函数或类实现****************************************/

// sysfs 属性的显示函数
static ssize_t my_data_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", my_data);
}


// sysfs 属性的存储函数
static ssize_t my_data_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int i;
    const char *param;

    snprintf(my_data, sizeof(my_data), "%s", buf);
    printk(KERN_INFO "Received data: %s\n", my_data);

    // 遍历命令处理函数数组
    for (i = 0; command_handlers[i].cmd; i++)
    {
        if (strncmp(my_data, command_handlers[i].cmd, strlen(command_handlers[i].cmd)) == 0)
        {
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