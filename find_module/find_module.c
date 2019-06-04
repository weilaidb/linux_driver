#include <linux/module.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include "../common/include/publicdef.h"
#include "../common/include/publicbasic.h"
#include "../common/include/dbgmsg.h"
#include "../common/dbgmsg.c"

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init find_module_init(void);
static void __exit find_module_exit(void);


static int __init find_module_init(void)
{
	const char *name = "test_module";
	struct module *fmodule = find_module(name);
	
	if(NULL != fmodule)
	{
		printk("fmodule->name  :%s\n", fmodule->name);
		printk("fmodule->state :%d\n", fmodule->state);
		printk("fmodule->percpu_size :%d\n", fmodule->percpu_size);
		printk("module_refcount(fmodule):%d\n", module_refcount(fmodule));
	}
	else
	{
		printk("fail to find  :%s\n", name);
	}
	
	name = "cuse";
	fmodule = find_module(name);
	if(NULL != fmodule)
	{
		printk("fmodule->name  :%s\n", fmodule->name);
		printk("fmodule->state :%d\n", fmodule->state);
		printk("fmodule->percpu_size :%d\n", fmodule->percpu_size);
		printk("module_refcount(fmodule):%d\n", module_refcount(fmodule));
	}
	else
	{
		printk("fail to find  :%s\n", name);
	}
	
	
    return 0;
}
//模块卸载函数
static void __exit find_module_exit(void)
{
    printk("module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(find_module_init);
module_exit(find_module_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

