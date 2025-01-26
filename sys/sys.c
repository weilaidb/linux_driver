#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>




// 创建一个 kobject 来表示 /sys/kernel/mymodule
static struct kobject *mymodule_kobj;

// 用于存储用户写入的内容
static char my_data[256] = {0};

// sysfs 属性的显示函数
static ssize_t my_data_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", my_data);
}

void my_function(void)
{
    struct task_struct *task;

    for_each_process(task)
    {
        printk(KERN_INFO "Process: %s (PID: %d)\n", task->comm, task->pid);
    }
}

void print_task_info(void)
{
    struct task_struct *task;

    for_each_process(task) // 遍历所有进程[^3^]
    {
        // 获取进程的 PID 和 TGID
        pid_t pid = task->pid; // 进程 ID[^3^]
        pid_t tgid = task->tgid; // 线程组 ID[^3^]

        // 获取进程的优先级
        int prio = task->prio; // 当前优先级[^3^]
        int static_prio = task->static_prio; // 静态优先级[^3^]
        int normal_prio = task->normal_prio; // 普通优先级[^3^]

        // 打印进程信息
        printk(KERN_INFO "Process: %s (PID: %d, TGID: %d, Prio: %d, Static Prio: %d, Normal Prio: %d)\n",
               task->comm, pid, tgid, prio, static_prio, normal_prio);
    }
}

// sysfs 属性的存储函数
static ssize_t my_data_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    snprintf(my_data, sizeof(my_data), "%s", buf);
    printk(KERN_INFO "Received data: %s\n", my_data);
    //如果数据为"showtask"，不包含\n, 则打印当前进程信息
    if(strncmp(my_data, "showtask", 8) == 0)
    {
        my_function();
    }
    //如果数据为"showalltask"，不包含\n, 则打印所有进程信息
    else if(strncmp(my_data, "showalltask", 11) == 0)
    {
        print_task_info();
    }

    return count;
}

// 定义 sysfs 属性
static struct kobj_attribute my_data_attr = __ATTR(my_data, 0644, my_data_show, my_data_store);

// 模块初始化函数
static int __init mymodule_init(void)
{
    int ret;

    // 创建一个 kobject 并将其添加到 /sys/kernel 下
    mymodule_kobj = kobject_create_and_add("mymodule", kernel_kobj);
    if (!mymodule_kobj) {
        printk(KERN_ERR "Failed to create kobject\n");
        return -ENOMEM;
    }

    // 创建 sysfs 属性
    ret = sysfs_create_file(mymodule_kobj, &my_data_attr.attr);
    if (ret) {
        kobject_put(mymodule_kobj);
        printk(KERN_ERR "Failed to create sysfs file\n");
        return ret;
    }

    printk(KERN_INFO "My module loaded.\n");
    return 0;
}

// 模块卸载函数
static void __exit mymodule_exit(void)
{
    sysfs_remove_file(mymodule_kobj, &my_data_attr.attr);
    kobject_put(mymodule_kobj);
    printk(KERN_INFO "My module unloaded.\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple module to create a sysfs entry under /sys/kernel");
MODULE_AUTHOR("Your Name");