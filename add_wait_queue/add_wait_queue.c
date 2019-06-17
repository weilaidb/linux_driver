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

int my_function(void *argc)
{
	printk("in the kernel thread function!\n");
	printk("the current pid is:%d\n", current->pid);	
	printk("out the kernel thread function\n");
	
	return 0;
}

static int __init  add_wait_queue_init(void);
static void __exit add_wait_queue_exit(void);

static int __init add_wait_queue_init(void)
{
	char namefrm[] = "add_wait_queue.c";
	char namefrm1[] = "add_wait_queue1.c";
	struct task_struct *result, *result1;
	int wait_queue_num = 0;
	wait_queue_head_t head;
	wait_queue_t data, data1, *curr, *next;
	printk("into add_wait_queue_init.\n");
	
	/** 创建2个新进程 **/
	result  = kthread_create_on_node(my_function, NULL, -1, namefrm);
	result1 = kthread_create_on_node(my_function, NULL, -1, namefrm1);
	init_waitqueue_head(&head); //初始化等待队列头指针
	/** 用新进程初始化等待队列元素**/
	init_waitqueue_entry(&data, result);
	init_waitqueue_entry(&data1, result1);
	
	/** 将新进程加入等待队列中 **/
	add_wait_queue(&head, &data);
	add_wait_queue(&head, &data1);
	__wake_up(&head, TASK_ALL, 0, NULL);
	
	/** 循环显示等待队列中的进程的信息 **/
	list_for_each_entry_safe(curr, next, &(head.task_list), task_list)
	{
		
	}
	
	
	
		
	//current pid, tgid, and out entry
	printk("the pid of current thread is:%d\n", current->pid);	
	printk("the current tgid is:%d\n", current->tgid);	
	printk("out add_wait_queue_init\n");
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit add_wait_queue_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(add_wait_queue_init);
module_exit(add_wait_queue_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

