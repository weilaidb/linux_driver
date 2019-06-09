#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kallsyms.h>
#include <linux/mm_types.h>
#include <linux/sched/mm.h>
#include <linux/pid_namespace.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init ns_of_pid_init(void);
static void __exit ns_of_pid_exit(void);

static int __init ns_of_pid_init(void)
{
	printk("into ns_of_pid_init.\n");
	
	struct pid * kpid = find_get_pid(current->pid);
	printk("the level of the pid is:%d\n", kpid->level);
	printk("the pid of the pid is:%d\n", kpid->numbers[kpid->level].nr);
	struct pid_namespace *pid_ns =  ns_of_pid(kpid);
	printk("the level of the pid_namespace is:%d\n", pid_ns->level);

	printk("the hide_pid of the pid_namespace is:%d\n", pid_ns->hide_pid);
	
	printk("the current PID  is:%d\n", current->pid);	
	printk("the current tgid is:%d\n", current->tgid);	
	printk("out ns_of_pid_init\n");
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit ns_of_pid_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(ns_of_pid_init);
module_exit(ns_of_pid_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

