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
static int __init find_symbol_init(void);
static void __exit find_symbol_exit(void);


static int __init find_symbol_init(void)
{
	const char *name = "perf_pmu_register";
	struct kernel_symbol *ksymbol;
	struct module *owner;
	const unsigned long *crc;
	bool gplok = true;
	bool warn = true;
	
	// ksymbol = find_symbol(name, &owner, &crc, gplok, warn);
	ksymbol = find_symbol(name, &owner, NULL, gplok, warn);
	if(NULL != ksymbol)
	{
		printk("ksymbol->value:%lx\n", ksymbol->value);
		printk("ksymbol->name :%lx\n", ksymbol->name);
	}
	else
	{
		printk("Failed to find symbol %s\n", name);
	}
	
	if(NULL != owner)
	{
		printk("owner->name  :%s\n", owner->name);
	}

	if(NULL != crc)
	{
		printk("*crc  :%lx\n", *crc);
	}

	
    return 0;
}
//模块卸载函数
static void __exit find_symbol_exit(void)
{
    printk("module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(find_symbol_init);
module_exit(find_symbol_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

