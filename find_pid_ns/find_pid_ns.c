#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kallsyms.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init find_pid_ns_init(void);
static void __exit find_pid_ns_exit(void);

static int __init find_pid_ns_init(void)
{
	int result;
	printk("into find_pid_ns_init.\n");
	
	struct pid * kpid = find_get_pid(current->pid);
	struct pid *fpid = find_pid_ns(kpid->numbers[kpid->level].nr
					, kpid->numbers[kpid->level].ns);
	printk("the find_pid_ns result's count is:%d\n", kpid->count);
	printk("the find_pid_ns result's level is:%d\n", kpid->count);
	printk("the find_pid_ns result's pid   is:%d\n", kpid->numbers[kpid->level].nr);
	printk("the pid of current thread      is:%d\n", current->pid);
	
	printk("out find_pid_ns_init\n");
	
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit find_pid_ns_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(find_pid_ns_init);
module_exit(find_pid_ns_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

