#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kallsyms.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init find_get_pid_init(void);
static void __exit find_get_pid_exit(void);

static int __init find_get_pid_init(void)
{
	printk("into find_get_pid_init.\n");
	
	struct pid * kpid = find_get_pid(current->pid);
	printk("the count of the pid is:%d\n", kpid->count);
	printk("the level of the pid is:%d\n", kpid->level);
	printk("the pid of the find_get_pid is:%d\n", kpid->numbers[kpid->level].nr);
	
	printk("out find_get_pid_init\n");
	
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit find_get_pid_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(find_get_pid_init);
module_exit(find_get_pid_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

