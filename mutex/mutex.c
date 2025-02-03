// mutex_example.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define DEVICE_NAME "mutex_example"

static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *pos);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write};

static int major;
static struct cdev my_cdev;
static char *shared_resource;
static DEFINE_MUTEX(resource_mutex);

static int __init my_init(void)
{
    int ret;
    dev_t dev_id;

    // 动态分配设备号
    ret = alloc_chrdev_region(&dev_id, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        pr_err("Failed to allocate character device region\n");
        return ret;
    }

    major = MAJOR(dev_id);
    pr_info("Major number: %d\n", major);

    // 初始化cdev
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    // 添加cdev
    ret = cdev_add(&my_cdev, dev_id, 1);
    if (ret < 0)
    {
        unregister_chrdev_region(dev_id, 1);
        pr_err("Failed to add character device\n");
        return ret;
    }

    // 分配共享资源
    shared_resource = kmalloc(1024, GFP_KERNEL);
    if (!shared_resource)
    {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_id, 1);
        pr_err("Failed to allocate shared resource\n");
        return -ENOMEM;
    }

    pr_info("Driver loaded successfully\n");
    return 0;
}

static void __exit my_exit(void)
{
    dev_t dev_id = MKDEV(major, 0);

    // 释放共享资源
    kfree(shared_resource);

    // 删除cdev
    cdev_del(&my_cdev);

    // 注销设备号
    unregister_chrdev_region(dev_id, 1);

    pr_info("Driver unloaded successfully\n");
}

static int my_open(struct inode *inode, struct file *file)
{
    pr_info("Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    pr_info("Device closed\n");
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    int ret;

    // 加锁
    mutex_lock(&resource_mutex);

    // 将数据复制到用户空间
    ret = copy_to_user(buf, shared_resource, min(count, 1024UL));
    if (ret == 0)
    {
        pr_info("Read successful:%s\n", shared_resource);
        mutex_unlock(&resource_mutex);
        return 1024;
    }
    else
    {
        pr_err("Failed to copy data to user space\n");
        mutex_unlock(&resource_mutex);
        return -EFAULT;
    }
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    int ret;

    // 加锁
    mutex_lock(&resource_mutex);

    // 从用户空间复制数据
    ret = copy_from_user(shared_resource, buf, min(count, 1024UL));
    if (ret == 0)
    {
        pr_info("Write successful:%s\n", shared_resource);
        mutex_unlock(&resource_mutex);
        return count;
    }
    else
    {
        pr_err("Failed to copy data from user space\n");
        mutex_unlock(&resource_mutex);
        return -EFAULT;
    }
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple mutex example driver");
MODULE_VERSION("0.1");