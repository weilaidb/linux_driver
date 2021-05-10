#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/list.h>

extern struct task_struct init_task;
//模块加载函数
static int print_pid_init(void)
{
    printk(KERN_ALERT "print_pid,\n");
	struct task_struct *task, *p;
	struct list_head *pos;
	int count = 0;
	printk("Hello World enter begin:\n");
	printk("%3s %10s\n", "PID", "NAME");
	task = &init_task;
	list_for_each(pos, &task->tasks)
	{
		p = list_entry(pos, struct task_struct, tasks);
		count++;
		printk("%d--->%s\n", p->pid, p->comm);
	}
	printk("the number of process is:%d\n", count);

    return 0;
}
//模块卸载函数
static void print_pid_exit(void)
{
    printk(KERN_ALERT "goodbye,kernel\n");
}
//模块注册
module_init(print_pid_init);
module_exit(print_pid_exit);
//必选
//模块许可声明
MODULE_LICENSE("GPL");
//可选
MODULE_AUTHOR("edsionte Wu");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

