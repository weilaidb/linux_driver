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
static int __init  task_active_pid_ns_init(void);
static void __exit task_active_pid_ns_exit(void);

static int __init task_active_pid_ns_init(void)
{
	printk("into task_active_pid_ns_init.\n");
	
	struct pid * kpid = find_get_pid(current->pid);
	struct task_struct *task = pid_task(kpid, PIDTYPE_PID);
	struct pid_namespace *ns = task_active_pid_ns(task);
	
	printk("the count of the pid is:%d\n", kpid->count);
	printk("the level of the pid is:%d\n", kpid->level);
	printk("the pid of the pid is:%d\n", kpid->numbers[kpid->level].nr);
	printk("the hide_pid of the task_active_pid_ns result is:%d\n"
			, ns->hide_pid);
		
	//current pid, tgid, and out entry
	printk("the current pid  is:%d\n", current->pid);	
	printk("the current tgid is:%d\n", current->tgid);	
	printk("out task_active_pid_ns_init\n");
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit task_active_pid_ns_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(task_active_pid_ns_init);
module_exit(task_active_pid_ns_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

