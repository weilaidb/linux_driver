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
	
	symbol_name = "cpu_debug_store";
	addr = __symbol_get(symbol_name);
	
	if(NULL != addr)
		printk("the address of %s is :%lx\n", symbol_name, (unsigned long)addr);
	else
		printk("%s isn't found\n", symbol_name);
	
	symbol_name = "symbol_0";
	addr = __symbol_get(symbol_name);
	
	if(NULL != addr)
		printk("the address of %s is :%lx\n", symbol_name, (unsigned long)addr);
	else
		printk("%s isn't found\n", symbol_name);
	
 	symbol_name = "xor";
	addr = __symbol_get(symbol_name);
	
	if(NULL != addr)
		printk("the address of %s is :%lx\n", symbol_name, (unsigned long)addr);
	else
		printk("%s isn't found\n", symbol_name);
	
 	symbol_name = "DbgPrintMsg";
	addr = __symbol_get(symbol_name);
	
	if(NULL != addr)
		printk("the address of %s is :%lx\n", symbol_name, (unsigned long)addr);
	else
		printk("%s isn't found\n", symbol_name);
	
 	symbol_name = "cpu_info";
	addr = __symbol_get(symbol_name);
	
	if(NULL != addr)
		printk("the address of %s is :%lx\n", symbol_name, (unsigned long)addr);
	else
		printk("%s isn't found\n", symbol_name);
	
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

