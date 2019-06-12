#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include <linux/mm_types.h>
#include <linux/sched/mm.h>
#include <linux/pid_namespace.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");

static wait_queue_head_t head; //等待队列头元素
struct task_struct *old_thread;//保存进程描述符信息

int my_function(void *argc)
{
	printk("in the kernel thread function!\n");
	printk("the current pid is:%d\n", current->pid);
	
	printk("the state of the init function is:%d\n", old_thread->state);
	__wake_up(&head, TASK_ALL, 0, NULL);
	
	printk("out the kernel thread function\n");
	
	return 0;
}

static int __init  __wake_up_init(void);
static void __exit __wake_up_exit(void);

static int __init __wake_up_init(void)
{
	printk("into __wake_up_init.\n");
	char namefrm[] = "__wake_up.c%s";
	long time_out;
	struct task_struct *result;
	// wait_queue_t data;
	struct wait_queue_entry data;
	result = kthread_create_on_node(my_function, NULL, -1, namefrm);
	printk("the pid of the new thread is:%d\n", result->pid);
	printk("the current pid is:%d\n", current->pid);
	init_waitqueue_head(&head);
	init_waitqueue_entry(&data, current);
	add_wait_queue(&head, &data);
	old_thread = current;
	wake_up_process(result);
	time_out = schedule_timeout_uninterruptible(1000 * 10);
	printk("the schedule timeout is:%ld\n", time_out);
	printk("out __wake_up_init.\n");
	
		
	//current pid, tgid, and out entry
	printk("the pid of current thread is:%d\n", current->pid);	
	printk("the current tgid is:%d\n", current->tgid);	
	printk("out __wake_up_init\n");
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit __wake_up_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(__wake_up_init);
module_exit(__wake_up_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

