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

static int __init  abort_exclusive_wait_init(void);
static void __exit abort_exclusive_wait_exit(void);

static int __init abort_exclusive_wait_init(void)
{
	struct task_struct *result, *result1, *result2;
	char namefrm[] = "abort_exclusive_wait.c";
	char namefrm1[] = "abort_exclusive_wait1.c";
	char namefrm2[] = "abort_exclusive_wait2.c";
	int wait_queue_num = 0;
	wait_queue_head_t head; //等待队列头元素
	
	// wait_queue_t data;
	// struct wait_queue_entry data, data1, data2, *curr, *next;
	// wait_queue_t data, data1, data2, *curr, *next;
	wait_queue_entry_t data, data1, data2, *curr, *next;
	printk("into abort_exclusive_wait_init.\n");
	
	result  = kthread_create_on_node(my_function, NULL, -1, namefrm);
	result1 = kthread_create_on_node(my_function, NULL, -1, namefrm1);
	result2 = kthread_create_on_node(my_function, NULL, -1, namefrm2);
	wake_up_process(result);
	wake_up_process(result1);
	wake_up_process(result2);
	init_waitqueue_head(&head);
	init_waitqueue_entry(&data, result);
	data.entry.next = &data.entry;
	prepare_to_wait(&head, &data, 130);
	
	init_waitqueue_entry(&data1, result1);
	data1.entry.next = &data1.entry;
	prepare_to_wait_exclusive(&head, &data1, 2);
	
	init_waitqueue_entry(&data2, result2);
	data2.entry.next = &data2.entry;
	prepare_to_wait_exclusive(&head, &data2, 1);
	
	list_for_each_entry_safe(curr, next, &(head.head), entry)
	{
		wait_queue_num++;
		printk("the pid value of the current data of the waitqeue is:%d\n",
				((struct task_struct *)(curr->private))->pid);
		printk("the state of the current data of the waitqueue is:%ld\n",
				((struct task_struct *)(curr->private))->state);
	}
	printk("the value of the wait_queue_num is:%d\n", wait_queue_num);
	printk("the state of the current thread is:%ld\n", current->state);
	
	/**
	((struct task_struct *)(data2.private))->state =130;
	finish_wait(&head, &data1);
	wait_queue_num = 0;
	list_for_each_entry_safe(curr, next, &(head.task_list), task_list)
	{
		wait_queue_num++;
		printk("the pid value of the current data of the waitqueue is:%d\n",
				((struct task_struct *)(curr->private))->pid);
		printk("the state of the current data of the waitqueue is:%d\n",
				((struct task_struct *)(curr->private))->state);
	}
	
	**/
	// abort_exclusive_wait(&head, &data1, TASK_NORMAL, NULL);
	abort_exclusive_wait_exit(&head, &data1, TASK_NORMAL, NULL);
	printk("the state of the current thread is :%ld\n", current->state);
	
	wait_queue_num = 0;
	list_for_each_entry_safe(curr, next, &(head.head), entry)
	{
		wait_queue_num++;
		printk("the pid value of the current data of the waitqueue is:%d\n",
				((struct task_struct *)(curr->private))->pid);
		printk("the state of the current data of the waitqueue is:%ld\n",
				((struct task_struct *)(curr->private))->state);
	}
	printk("the value of the wait_queue_num is:%d\n", wait_queue_num);
	
	
	
	
	
	printk("the pid of result is:%d\n", result->pid);
	printk("the pid of result1 is:%d\n", result1->pid);
	printk("the pid of result2 is:%d\n", result2->pid);
		
	//current pid, tgid, and out entry
	printk("the pid of current thread is:%d\n", current->pid);	
	printk("the current tgid is:%d\n", current->tgid);	
	printk("out abort_exclusive_wait_init\n");
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit abort_exclusive_wait_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(abort_exclusive_wait_init);
module_exit(abort_exclusive_wait_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

