#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kallsyms.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init find_vpid_init(void);
static void __exit find_vpid_exit(void);

static int __init find_vpid_init(void)
{
	printk("into find_vpid_init.\n");
	
	struct pid * vpid = find_vpid(current->pid);
	printk("the count of the pid           is:%d\n", vpid->count);
	printk("the level of the pid           is:%d\n", vpid->level);
	printk("the pid of the find_vpid       is:%d\n", vpid->numbers[vpid->level].nr);
	printk("the pid of current thread      is:%d\n", current->pid);
	
	printk("out find_vpid_init\n");
	
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit find_vpid_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(find_vpid_init);
module_exit(find_vpid_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

