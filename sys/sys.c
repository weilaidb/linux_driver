#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/slab.h>

// 创建一个 kobject 来表示 /sys/kernel/mymodule
static struct kobject *mymodule_kobj;

// 用于存储用户写入的内容
static char my_data[256] = {0};

// sysfs 属性的显示函数
static ssize_t my_data_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", my_data);
}

// sysfs 属性的存储函数
static ssize_t my_data_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    snprintf(my_data, sizeof(my_data), "%s", buf);
    printk(KERN_INFO "Received data: %s\n", my_data);
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