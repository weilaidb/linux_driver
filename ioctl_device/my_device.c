#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "my_device"
#define IOCTL_SET_VALUE _IOR('k', 0, int)
#define IOCTL_GET_VALUE _IOR('k', 1, int)

static int value = 0;
static dev_t dev;
static struct cdev cdev;
static struct class *class;
static struct device *device;

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    int tmp;

    switch (cmd)
    {
        case IOCTL_SET_VALUE:
            if (copy_from_user(&tmp, (int *)arg, sizeof(int)))
                return -EFAULT;
            value = tmp;
            break;

        case IOCTL_GET_VALUE:
            if (copy_to_user((int *)arg, &value, sizeof(int)))
                return -EFAULT;
            break;

        default:
            return -ENOTTY;
    }

    return ret;
}

static struct file_operations fops = {
    .unlocked_ioctl = my_ioctl,
};

static int __init my_driver_init(void)
{
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0)
        return -1;

    cdev_init(&cdev, &fops);
    if (cdev_add(&cdev, dev, 1) == -1)
    {
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    class = class_create(THIS_MODULE, DEVICE_NAME);
    device = device_create(class, NULL, dev, NULL, DEVICE_NAME);

    return 0;
}

static void __exit my_driver_exit(void)
{
    device_destroy(class, dev);
    class_unregister(class);
    class_destroy(class);
    cdev_del(&cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(my_driver_init);
module_exit(my_driver_exit);
MODULE_LICENSE("GPL");