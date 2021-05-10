#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/list.h>

extern struct task_struct init_task;
//static int __init pid_vnr_init(pid_t nr)
//{
//	printk("into pid_vnr_init.\n");

//	struct pid * kpid = find_get_pid(nr);

//	printk("the level of the pid is:%d\n", kpid->level);
//	printk("the pid of the pid is:%d\n", kpid->numbers[kpid->level].nr);

//	int vnr = pid_vnr(kpid);
//	printk("the pid_vnr result is:%d\n", vnr);

//	//current pid, tgid, and out entry
//	printk("the current pid  is:%d\n", nr);
////	printk("the current tgid is:%d\n", current->tgid);
//	printk("out pid_vnr_init\n");
//	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
//    return 0;
//}

//模块加载函数
static int print_pid_init(void)
{
    printk(KERN_ALERT "print_pid,\n");
	struct task_struct *task, *p;
	struct list_head *pos;
	int count = 0;
	printk("Hello World enter begin:\n");
	printk("%-6s %-30s %-10s %-10s %-10s\n", "PID", "NAME", "LEVEL", "NR", "VNR");
	task = &init_task;
	list_for_each(pos, &task->tasks)
	{
		p = list_entry(pos, struct task_struct, tasks);
		count++;

		//other info
		struct pid * kpid = find_get_pid(p->pid);
		int vnr = pid_vnr(kpid);

		printk("%-6d--->%-30s %-10d %-10d %-10d\n"
					, p->pid, p->comm
					, kpid->level
					, kpid->numbers[kpid->level].nr
					, vnr
					);
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

