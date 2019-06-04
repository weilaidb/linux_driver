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
static int __init __symbol_put_init(void);
static void __exit __symbol_put_exit(void);


static int __init __symbol_put_init(void)
{
	const char * symbol_name;
	const char * mod_name;
	struct module * fmodule;
	
	symbol_name = "parport";
	mod_name = "parport";
	fmodule = find_module(mod_name);
	
	if(NULL != fmodule)
	{
		printk("before calling __symbol_put,\n");
		printk("ref of %s is :%d\n", mod_name, module_refcount(fmodule));
		__symbol_put(symbol_name);
		printk("after calling __symbol_put, \n");
		printk("ref of %s is:%d\n", mod_name, module_refcount(fmodule));
	}
	else
	{
		printk("find %s failed!\n", mod_name);
	}
	
    return 0;
}
//模块卸载函数
static void __exit __symbol_put_exit(void)
{
    printk("module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(__symbol_put_init);
module_exit(__symbol_put_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

