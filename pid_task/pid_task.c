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
static int __init  pid_task_init(void);
static void __exit pid_task_exit(void);

static int __init pid_task_init(void)
{
	printk("into pid_task_init.\n");
	
	struct pid * kpid = find_get_pid(current->pid);
	struct task_struct *task = pid_task(kpid, PIDTYPE_PID);
	
	printk("the state of the task is:%d\n", task->state);
	printk("the pid  of the task is:%d\n", task->pid);
	printk("the tgid of the task is:%d\n", task->tgid);

	//current pid, tgid, and out entry
	printk("the current pid  is:%d\n", current->pid);	
	printk("the current tgid is:%d\n", current->tgid);	
	printk("out pid_task_init\n");
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit pid_task_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(pid_task_init);
module_exit(pid_task_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

