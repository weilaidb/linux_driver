#include <linux/module.h>
#include <linux/init.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init module_is_live_init(void);
static void __exit module_is_live_exit(void);

static int ret;
static int __init module_is_live_init(void)
{
	ret = module_is_live(THIS_MODULE);
	
	if(1 == ret)
	{
		printk("<0>in init, state is : not GOING!\n");		
	}
	else
	{
		printk("<0>in init, state is : GOING!\n");		
	}

    return 0;
}
//模块卸载函数
static void __exit module_is_live_exit(void)
{
	ret = module_is_live(THIS_MODULE);
	
	if(1 == ret)
	{
		printk("<0>in exit, state is : not GOING!\n");		
	}
	else
	{
		printk("<0>in exit, state is : GOING!\n");		
	}
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(module_is_live_init);
module_exit(module_is_live_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

