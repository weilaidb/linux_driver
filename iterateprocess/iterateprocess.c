/**
** 遍历进程表
**/
#include<linux/init.h>
#include<linux/module.h>
#include<linux/sched.h>
#include<linux/sem.h>
#include<linux/list.h>
//必选
//模块许可声明
MODULE_LICENSE("GPL");
//模块加载函数
static int __init  traverse_init(void)
{
	struct task_struct *pos;
	struct list_head *current_head;
	int count=0;

	printk("Traversal module is working..\n");
	current_head=&(current->tasks);
	list_for_each_entry(pos,current_head,tasks)
	{
		count++;
		printk("[process %-5d]: %s\'s pid is %-5d\n",count,pos->comm,pos->pid);
	}
	printk(KERN_ALERT"The number of process is:%-5d\n",count);
	return 0;
}
//模块卸载函数
static void __exit traverse_exit(void)
{
	printk("traverse  exit\n");
}

//模块注册
module_init(traverse_init);
module_exit(traverse_exit);
//可选
MODULE_AUTHOR("edsionte Wu");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

