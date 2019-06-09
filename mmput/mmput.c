#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kallsyms.h>
#include <linux/mm_types.h>
#include <linux/sched/mm.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init mmput_init(void);
static void __exit mmput_exit(void);

static int __init mmput_init(void)
{
	printk("into mmput_init.\n");
	
	struct pid * kpid = find_get_pid(current->pid);
	struct task_struct *task = pid_task(kpid, PIDTYPE_PID);
	struct mm_struct *mm_task  = get_task_mm(task); 
	/** 
	** 显示mm_task字段mm_users和字段mm_count的值
	**/ 
	printk("the mm_users of the mm_struct is:%d\n", mm_task->mm_users);
	printk("the mm_count of the mm_struct is:%d\n", mm_task->mm_count);

	/**
	** 显示与此mm_task相关进程的父进程的TGID和PID号
	**/
	printk("the tgid of the mm_struct is:%d\n", mm_task->owner->tgid);
	printk("the pid of the mm_struct is:%d\n", mm_task->owner->pid);
	mmput(mm_task);
	printk("the new value of the mm_struct after the function mmput:\n");
	printk("the mm_users of the mm_struct is:%d\n", mm_task->mm_users);
	printk("the mm_count of the mm_struct is:%d\n", mm_task->mm_count);
	printk("the tgid of the mm_struct is:%d\n", mm_task->owner->tgid);
	printk("the pid of the mm_struct is:%d\n", mm_task->owner->pid);
	printk("the current PID is:%d\n", current->pid);
	
	printk("out mmput_init\n");
	
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit mmput_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(mmput_init);
module_exit(mmput_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

