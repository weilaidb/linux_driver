#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kallsyms.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init __task_pid_nr_ns_init(void);
static void __exit __task_pid_nr_ns_exit(void);

static int __init __task_pid_nr_ns_init(void)
{
	printk("into __task_pid_nr_ns_init.\n");
	
	struct pid * kpid = find_get_pid(current->pid);
	
	struct task_struct * task = pid_task(kpid, PIDTYPE_PID);
	
	pid_t result1 = __task_pid_nr_ns(task, PIDTYPE_PID, kpid->numbers[kpid->level].ns);
	
	printk("the result of the __task_pid_nr_ns is:%d\n", result1);
	printk("the pid of current thread is:%d\n", current->pid);
	printk("out __task_pid_nr_ns\n");
	
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit __task_pid_nr_ns_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(__task_pid_nr_ns_init);
module_exit(__task_pid_nr_ns_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

