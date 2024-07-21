#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/io.h>
//#include <linux/io_address.h>
// #include <linux/io_irq.h>
#include <linux/cdev.h>
#include <linux/fs.h>


#define DTSLED_CNT 1
#define DTSLED_NAME "dtsled"

static int __init dtsofled_init(void);
static void __exit dtsofled_exit(void);

struct dtsled_dev {
    dev_t devid; /*设备号*/
    struct cdev cdev;/*字符设备*/
    struct device *device; /*设备结构体*/
    struct class *class; /*类指针*/
    int major; /*主设备号*/
    int minor; /*次设备号*/
}; /*定义dtsled设备结构体*/

struct dtsled_dev dtsled; /*led设备*/

//static int dtsled_open(struct file *filp)
//{
//
//}
//

/*dtsled 字符设备操作集*/
static const struct file_operations dtsled_fops = {
    .owner = THIS_MODULE,
    // .open = dtsled_open,

};

static int __init dtsofled_init(void)
{
    int ret = 0;

    /*注册字符设备*/
    dtsled.major = 0; /* 设备号*/
    if(dtsled.major) {
        dtsled.devid = MKDEV(dtsled.major, 0);
        ret = register_chrdev_region(dtsled.devid, DTSLED_CNT, DTSLED_NAME);
    } else {
        ret = alloc_chrdev_region(&dtsled.devid, 0,DTSLED_CNT, DTSLED_NAME);
        dtsled.major = MAJOR(dtsled.devid);
        dtsled.minor = MINOR(dtsled.devid);
    }
    if(ret < 0) {
        goto fail_devid;
    }

    /*2.添加字符设备*/
    dtsled.cdev.owner = THIS_MODULE;
    cdev_init(&dtsled.cdev, &dtsled_fops);
    ret = cdev_add(&dtsled.cdev, dtsled.devid, DTSLED_CNT);
    if(ret < 0) {
        goto fail_cdev;
    }
    /*3、自动创建设备节点*/
    dtsled.class = class_create(THIS_MODULE, DTSLED_NAME);
    if(IS_ERR(dtsled.class)) {
        ret = PTR_ERR(dtsled.class);
        goto fail_class;
    }

    dtsled.device = device_create(dtsled.class, NULL, dtsled.devid, NULL, DTSLED_NAME);
    if(IS_ERR(dtsled.device)) {
        ret = PTR_ERR(dtsled.device);
        goto fail_device;
    }


    return 0;

fail_device:
    device_destroy(dtsled.class, dtsled.devid);
fail_class:
    cdev_del(&dtsled.cdev);
fail_cdev:
    unregister_chrdev_region(dtsled.devid, DTSLED_CNT);
fail_devid:
    return ret;
}
//模块卸载函数
static void __exit dtsofled_exit(void)
{
    /* 删除字符设备*/
    cdev_del(&dtsled.cdev);
    unregister_chrdev_region(dtsled.devid, DTSLED_CNT);
    device_destroy(dtsled.class, dtsled.devid);
    class_destroy(dtsled.class);
    printk(KERN_ALERT "dtsofled exit ok!\n");
}




//模块注册
module_init(dtsofled_init);
module_exit(dtsofled_exit);
//必选
//模块许可声明
MODULE_LICENSE("GPL");
//可选
MODULE_AUTHOR("dtsofled");
MODULE_DESCRIPTION("dtsofled simple example!\n");
MODULE_ALIAS("dtsofled simplest example");

