#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

// 用于存储用户写入的内容
static char user_data[256] = {0};

// seq_file 的显示函数
static int my_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "User data: %s\n", user_data);
    return 0;
}

// 打开文件时调用的函数
static int my_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_proc_show, NULL);
}

// 写入文件时调用的函数
static ssize_t my_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    if (count >= sizeof(user_data))
        return -EFAULT; // 防止溢出

    if (copy_from_user(user_data, buf, count))
        return -EFAULT;

    user_data[count] = '\0'; // 确保字符串以 null 结尾
    printk(KERN_INFO "Received data: %s\n", user_data);
    return count;
}

// proc_ops 结构体
static const struct proc_ops my_proc_ops = {
    .proc_open = my_proc_open,
    .proc_read = seq_read,
    .proc_write = my_proc_write,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// 模块初始化函数
static int __init my_module_init(void)
{
    struct proc_dir_entry *entry;

    // 创建 /proc/sys/my_module 节点
    entry = proc_create("sys/my_module", 0666, NULL, &my_proc_ops);
    if (!entry) {
        printk(KERN_ERR "Failed to create /proc/sys/my_module\n");
        // 打印可能的原因
        printk(KERN_ERR "Possible reasons:\n");
        printk(KERN_ERR "1. Insufficient memory\n");
        printk(KERN_ERR "2. Invalid parameters\n");
        printk(KERN_ERR "3. Permission issues\n");
        printk(KERN_ERR "4. Kernel configuration issues\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "My module loaded.\n");
    return 0;
}

// 模块卸载函数
static void __exit my_module_exit(void)
{
    remove_proc_entry("sys/my_module", NULL);
    printk(KERN_INFO "My module unloaded.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple module to create a proc entry in /proc/sys");
MODULE_AUTHOR("Your Name");


