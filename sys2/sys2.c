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
#include <linux/kallsyms.h>

// 创建一个 kobject 来表示 /sys/kernel/mymodule
static struct kobject *mymodule_kobj;

// 用于存储用户写入的内容
static char my_data[256] = {0};

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