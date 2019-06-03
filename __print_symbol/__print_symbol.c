#include <linux/module.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include "../common/include/publicdef.h"
#include "../common/include/publicbasic.h"
#include "../common/include/dbgmsg.h"
#include "../common/dbgmsg.c"

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init __print_symbol_init(void);
static void __exit __print_symbol_exit(void);
//模块加载函数
int a_symbol(void)
{
	return 0;
}

static int __init __print_symbol_init(void)
{
	char *fmt;
	unsigned long address;
	char *name;
	struct module * fmodule = NULL;
	
	address = (unsigned long) __builtin_return_address(0);
	fmt = "it's the first part,\n %s";
	__print_symbol(fmt, address);
	printk("\n\n");
	
	name = "psmouse";
	fmodule = find_module(name);
	
	if(fmodule != NULL)
	{
		printk("fmodule->name:%s\n", fmodule->name);
		address = (unsigned long)fmodule->core_layout.base;
		// address = 0;
		fmt = "it's the second part, \n %s";
		__print_symbol(fmt, address);
	}
	printk("\n\n");
	
	address = (unsigned long)a_symbol + 5;
	fmt = "it's the third part,\n %s";
	__print_symbol(fmt, address);
	
	printk("\n\n");
	
    return 0;
}
//模块卸载函数
static void __exit __print_symbol_exit(void)
{
    printk("module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(__print_symbol_init);
module_exit(__print_symbol_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

