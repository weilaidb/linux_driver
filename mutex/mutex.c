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
    .write = my_write
};

static int major;
static struct cdev my_cdev;
static char *shared_resource;
static DEFINE_MUTEX(resource_mutex);

static int __init my_init(void)
{
    int ret;

    // Allocate a character device
    ret = alloc_chrdev_region(&major, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate character device region\n");
        return ret;
    }

    // Initialize cdev
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    // Register cdev
    ret = cdev_add(&my_cdev, MKDEV(major, 0), 1);
    if (ret < 0) {
        unregister_chrdev_region(MKDEV(major, 0), 1);
        pr_err("Failed to add character device\n");
        return ret;
    }

    // Allocate shared resource
    shared_resource = kmalloc(1024, GFP_KERNEL);
    if (!shared_resource) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(MKDEV(major, 0), 1);
        pr_err("Failed to allocate shared resource\n");
        return -ENOMEM;
    }

    pr_info("Driver loaded successfully\n");
    return 0;
}

static void __exit my_exit(void)
{
    // Free shared resource
    kfree(shared_resource);

    // Remove cdev
    cdev_del(&my_cdev);

    // Unregister character device region
    unregister_chrdev_region(MKDEV(major, 0), 1);

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

    // Lock the resource
    mutex_lock(&resource_mutex);

    // Copy data to user space
    ret = copy_to_user(buf, shared_resource, min(count, 1024UL));
    if (ret == 0) {
        pr_info("Read successful\n");
        mutex_unlock(&resource_mutex);
        return 1024;
    } else {
        pr_err("Failed to copy data to user space\n");
        mutex_unlock(&resource_mutex);
        return -EFAULT;
    }
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    int ret;

    // Lock the resource
    mutex_lock(&resource_mutex);

    // Copy data from user space
    ret = copy_from_user(shared_resource, buf, min(count, 1024UL));
    if (ret == 0) {
        pr_info("Write successful\n");
        mutex_unlock(&resource_mutex);
        return count;
    } else {
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


