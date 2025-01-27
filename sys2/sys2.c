#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/pid.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/sched.h>
#include <linux/cred.h>

// 创建一个 kobject 来表示 /sys/kernel/mymodule
static struct kobject *mymodule_kobj;

// 用于存储用户写入的内容
static char my_data[256] = {0};

// sysfs 属性的显示函数
static ssize_t my_data_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", my_data);
}

void showtasklist(void)
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

void print_task_info(void)
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

// 设置指定用户的进程的优先级
static void set_user_nice_by_username(const char *username, int nice_value)
{
    struct task_struct *task;
    struct user_namespace *user_ns = current_user_ns();
    struct user_struct *user;

    user = find_user_by_name(user_ns, username);
    if (!user) {
        printk(KERN_ERR "User '%s' not found.\n", username);
        return;
    }

    for_each_process(task)
    {
        if (task->pid != 0 && task_uid(task) == user->uid) // 跳过 init 进程
        {
            if (set_user_nice(task, nice_value) == 0)
            {
                printk(KERN_INFO "Set nice value of process %s (PID: %d, UID: %u) to %d\n", task->comm, task->pid, user->uid, nice_value);
            }
            else
            {
                printk(KERN_INFO "Failed to set nice value of process %s (PID: %d, UID: %u) to %d\n", task->comm, task->pid, user->uid, nice_value);
            }
        }
    }

    put_user_struct(user);
}

// sysfs 属性的存储函数
static ssize_t my_data_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    snprintf(my_data, sizeof(my_data), "%s", buf);
    printk(KERN_INFO "Received data: %s\n", my_data);

    // 如果数据为 "showtask"，不包含换行符，则打印当前进程信息
    if (strncmp(my_data, "showtask", 8) == 0)
    {
        showtasklist();
    }
    // 如果数据为 "showalltask"，不包含换行符，则打印所有进程信息
    else if (strncmp(my_data, "showalltask", 11) == 0)
    {
        print_task_info();
    }
    // 如果数据为 "parse"，则解析命令和参数
    else if (strncmp(my_data, "parse", 5) == 0)
    {
        parse_command(my_data + 6); // 跳过 "parse" 命令本身
    }
    // 如果数据为 "setnice"，则设置所有进程的优先级
    else if (strncmp(my_data, "setnice", 7) == 0)
    {
        int nice_value;
        if (sscanf(my_data + 8, "%d", &nice_value) == 1)
        {
            set_all_task_nice(nice_value);
        }
        else
        {
            printk(KERN_ERR "Invalid nice value\n");
        }
    }
    // 如果数据为 "setniceuser"，则设置指定用户的进程的优先级
    else if (strncmp(my_data, "setniceuser", 11) == 0)
    {
        char username[64];
        int nice_value;
        if (sscanf(my_data + 12, "%63s %d", username, &nice_value) == 2)
        {
            set_user_nice_by_username(username, nice_value);
        }
        else
        {
            printk(KERN_ERR "Invalid input format. Expected: setniceuser <username> <nice_value>\n");
        }
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