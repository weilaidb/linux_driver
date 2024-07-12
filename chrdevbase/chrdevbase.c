/***************************************************************************
** 版权所有:   Copyright (c) 2020-2030
** 文件名称:  chrdevbase.c
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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

/*****************************宏定义****************************************/
#define CHARDEVBASE_MAJOR 200
#define CHARDEVBASE_NAME "chrdevbase"


/*****************************结构体或类型定义*******************************/

/*****************************全局变量****************************************/

/*****************************本地变量****************************************/
static char writebuf[100];
static char readbuf[100];
static char kerneldata[] = {"kernel data"};
static char samebuf[100];

/*****************************函数实现****************************************/
static int chrdevbase_open (struct inode *inode, struct file *filp)
{
    printk("chrdevbase_open\n");
    return 0;
}


static int chrdevbase_release (struct inode *inode, struct file *filp)
{
    printk("chrdevbase_release\n");
    return 0;
}
static ssize_t chrdevbase_read (struct file *filp, char __user *buf, size_t count, loff_t *p2)
{
    // printk("chrdevbase_read\n");
    int ret = 0;
    ret = copy_to_user(buf, samebuf, sizeof(samebuf));
    if( 0 == ret ) {
        printk("chrdevbase_read success\n");
    }
    return 0;
}

static ssize_t chrdevbase_write (struct file *filp, const char __user *buf, size_t count, loff_t *p2)
{
    int ret = 0;
//    printk("chrdevbase_write\n");
    ret = copy_from_user(samebuf, buf, count);
    if( 0 == ret ) {
        printk("chrdevbase_write success, count:%u\n", count);
    }
    return 0;
}

const struct file_operations chrdevbase_fops = {
    .owner = THIS_MODULE,
    .open = chrdevbase_open,
    .release = chrdevbase_release,
    .read = chrdevbase_read,
    .write = chrdevbase_write,
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
static int __init chrdevbase_init(void) {

    /*注册设备驱动*/
    int ret = register_chrdev(CHARDEVBASE_MAJOR, CHARDEVBASE_NAME,&chrdevbase_fops);
    if(ret < 0) {
        printk(KERN_INFO "entering  %s @ %i %d\n", __FUNCTION__, task_pid_nr(current), __LINE__);
        printk("chrdevbase register failed:%d\n", ret);
    }

    printk(KERN_INFO "ChardevBase driver has been loaded\n");

    return 0;
}

static void __exit chrdevbase_exit(void) {
    /*注销设备驱动 */
    unregister_chrdev(CHARDEVBASE_MAJOR, CHARDEVBASE_NAME);

    printk(KERN_INFO "ChardevBase driver has been unloaded\n");

}

module_init(chrdevbase_init);
module_exit(chrdevbase_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple example Linux driver");




#ifdef __cplusplus
}
#endif
