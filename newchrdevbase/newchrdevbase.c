/***************************************************************************
** 版权所有:   Copyright (c) 2020-2030
** 文件名称:  newchardevbase.c
** 当前版本:  v1.0
** 作    者:
** 完成日期: 2024-07-10
** 修改记录:
** 修改日期:
** 版本号  :
** 修改人  :
** 修改内容:
***************************************************************************/

/*****************************头文件****************************************/

/*****************************宏定义****************************************/

/*****************************结构体或类型定义*******************************/

/*****************************全局变量****************************************/

/*****************************本地变量****************************************/

/*****************************函数或类声明****************************************/

/*****************************函数或类实现****************************************/

/*****************************by extern "C"****************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*****************************头文件****************************************/
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/cdev.h>

/*****************************宏定义****************************************/
#define NEWCHARDEVBASE_MAJOR 200
#define NEWCHARDEVBASE_NAME "newchrled"
#define NEWCHRLED_COUNT 1


/*****************************结构体或类型定义*******************************/

/*****************************全局变量****************************************/

/*****************************本地变量****************************************/
static char samebuf[100];

/*****************************函数实现****************************************/
static int newchardevbase_open (struct inode *inode, struct file *filp)
{
    printk("newchardevbase_open\n");
    return 0;
}


static int newchardevbase_release (struct inode *inode, struct file *filp)
{
    printk("newchardevbase_release\n");
    return 0;
}
static ssize_t newchardevbase_read (struct file *filp, char __user *buf, size_t count, loff_t *p2)
{
    // printk("newchardevbase_read\n");
    int ret = 0;
    ret = copy_to_user(buf, samebuf, sizeof(samebuf));
    if( 0 == ret ) {
        printk("newchardevbase_read success\n");
    }
    return 0;
}

static ssize_t newchardevbase_write (struct file *filp, const char __user *buf, size_t count, loff_t *p2)
{
    int ret = 0;
//    printk("newchardevbase_write\n");
    ret = copy_from_user(samebuf, buf, count);
    if( 0 == ret ) {
        printk("newchardevbase_write success, count:%u\n", count);
    }
    return 0;
}

/* LED设备结构体 */
struct newchrled_cdev {
    struct cdev cdev; /*字符设备*/
    dev_t devid; /*设备号*/
    struct class  *class;/*类*/
    struct device *device;/*设备*/
    int major;  /*主设备号*/
    int minor;  /*次设备号*/
};

struct newchrled_cdev newchrled; /* led 设备*/

static const struct file_operations newchrled_fops = {
    .owner = THIS_MODULE,
    .open = newchardevbase_open,
    .release = newchardevbase_release,
    .read = newchardevbase_read,
    .write = newchardevbase_write,
};

/*
struct file_operations {
	struct module *owner;
	fop_flags_t fop_flags;
	loff_t (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
	ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
	int (*iopoll)(struct kiocb *kiocb, struct io_comp_batch *,
			unsigned int flags);
	int (*iterate_shared) (struct file *, struct dir_context *);
	__poll_t (*poll) (struct file *, struct poll_table_struct *);
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	int (*mmap) (struct file *, struct vm_area_struct *);
	int (*open) (struct inode *, struct file *);
	int (*flush) (struct file *, fl_owner_t id);
	int (*release) (struct inode *, struct file *);
	int (*fsync) (struct file *, loff_t, loff_t, int datasync);
	int (*fasync) (int, struct file *, int);
	int (*lock) (struct file *, int, struct file_lock *);
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
	int (*check_flags)(int);
	int (*flock) (struct file *, int, struct file_lock *);
	ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
	ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
	void (*splice_eof)(struct file *file);
	int (*setlease)(struct file *, int, struct file_lease **, void **);
	long (*fallocate)(struct file *file, int mode, loff_t offset,
			  loff_t len);
	void (*show_fdinfo)(struct seq_file *m, struct file *f);
#ifndef CONFIG_MMU
	unsigned (*mmap_capabilities)(struct file *);
#endif
	ssize_t (*copy_file_range)(struct file *, loff_t, struct file *,
			loff_t, size_t, unsigned int);
	loff_t (*remap_file_range)(struct file *file_in, loff_t pos_in,
				   struct file *file_out, loff_t pos_out,
				   loff_t len, unsigned int remap_flags);
	int (*fadvise)(struct file *, loff_t, loff_t, int);
	int (*uring_cmd)(struct io_uring_cmd *ioucmd, unsigned int issue_flags);
	int (*uring_cmd_iopoll)(struct io_uring_cmd *, struct io_comp_batch *,
				unsigned int poll_flags);
} __randomize_layout;
*/

/*****************************函数或类声明****************************************/

/*****************************函数或类实现****************************************/
static int __init newchardevbase_init(void) {
    int ret = 0;
    /*申请设备号*/

    /*注册字符设备*/
    if(newchrled.major) {
        newchrled.devid = MKDEV(newchrled.major, 0);
        ret = register_chrdev_region(newchrled.devid, NEWCHRLED_COUNT, NEWCHARDEVBASE_NAME);
    } else {
        ret = alloc_chrdev_region(&newchrled.devid, 0, NEWCHRLED_COUNT, NEWCHARDEVBASE_NAME);
        newchrled.major = MAJOR(newchrled.devid);
        newchrled.minor = MINOR(newchrled.devid);
    }
    if(ret < 0) {
        printk("newchardevbase alloc_chrdev_region failed:%d\n", ret);
        return -1;
    }

    printk("newchardevbase major:%d, minor:%d\n", newchrled.major, newchrled.minor);
    /*3.注册字符设备*/
    newchrled.cdev.owner = THIS_MODULE;
    cdev_init(&newchrled.cdev, &newchrled_fops);
    ret = cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLED_COUNT);

    /*注册设备驱动*/
    ret = register_chrdev(NEWCHARDEVBASE_MAJOR, NEWCHARDEVBASE_NAME,&newchrled_fops);
    if(ret < 0) {
        printk(KERN_INFO "entering  %s @ %i %d\n", __FUNCTION__, task_pid_nr(current), __LINE__);
        printk("newchardevbase register failed:%d\n", ret);
    }

    /*自动创建设备节点*/
    newchrled.class = class_create(THIS_MODULE, NEWCHARDEVBASE_NAME);
    if(IS_ERR(newchrled.class))
    {
        return PTR_ERR(newchrled.class);
    }
    newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, NULL, NEWCHARDEVBASE_NAME);
    if(IS_ERR(newchrled.device))
    {
        return PTR_ERR(newchrled.device);
    }

    printk(KERN_INFO "NewChardevBase driver has been loaded\n");

    return 0;
}

static void __exit newchardevbase_exit(void) {
    printk("newchrdevbase_exit");
    /*删除字符设备*/
    cdev_del(&newchrled.cdev);
    /*注销设备驱动 */
    unregister_chrdev_region(newchrled.devid, NEWCHRLED_COUNT);

    /*删除设备节点*/
    device_destroy(newchrled.class, newchrled.devid);

    /*删除设备类*/
    class_destroy(newchrled.class);
    printk(KERN_INFO "NewChardevBase driver has been unloaded\n");

}

module_init(newchardevbase_init);
module_exit(newchardevbase_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple example Linux driver");




#ifdef __cplusplus
}
#endif
