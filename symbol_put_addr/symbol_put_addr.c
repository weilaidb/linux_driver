#include <linux/module.h>
#include <linux/init.h>
#include <linux/kallsyms.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init symbol_put_addr_init(void);
static void __exit symbol_put_addr_exit(void);

static int __init symbol_put_addr_init(void)
{
	const char *symbol_name;
	const char *mod_name;
	struct module *fmodule = NULL;
	void *addr;
	
	symbol_name = "symbol_A";
	addr = (unsigned long)__symbol_get(symbol_name);
	if(NULL != addr)
	{
		printk("addr:%#x\n", (unsigned long)addr);
		
		mod_name = "test_module";
		fmodule = find_module(mod_name);
		if(NULL != NULL)
		{
			printk("before calling symbol_put_addr,\n");
			printk("refs of %s is: %d\n", mod_name, module_refcount(fmodule));
			symbol_put_addr(addr);
			printk("after calling symbol_put_addr,\n");
			printk("refs of %s is: %d\n", mod_name, module_refcount(fmodule));
		}
		else
		{
			printk("find %s failed!\n", mod_name);
		}
	}
	else
	{
		printk("%s isn't found\n", symbol_name);
	}
	
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit symbol_put_addr_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(symbol_put_addr_init);
module_exit(symbol_put_addr_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

