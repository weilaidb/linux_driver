#include <linux/module.h>
#include <linux/init.h>
#include <linux/kallsyms.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init sprint_symbol_init(void);
static void __exit sprint_symbol_exit(void);

int a_symbol(void)
{
	int a;
	return 1;
}

static int __init sprint_symbol_init(void)
{
	char buffer[KSYM_SYMBOL_LEN];
	int ret;
	unsigned long address;
	char *name;
	struct module *fmodule = NULL;
	address = (unsigned long)__builtin_return_address(0);
	
	ret = sprint_symbol(buffer, address);
	printk("KSYM_SYMBOL_LEN    :%u\n", KSYM_SYMBOL_LEN );
	printk("ret    :%d\n", ret );
	printk("buffer :%s\n", buffer );
	printk("\n");
	
	name = "test_module";
	fmodule = find_module(name);
	if(NULL != fmodule)
	{
		printk("fmodule->name:%s\n", fmodule->name);
		address = (unsigned long)fmodule->core_layout.base;
		ret = sprint_symbol(buffer, address);
		printk("ret    :%d\n", ret );
		printk("buffer :%s\n", buffer );
	}
	printk("\n");
	
	address = (unsigned long)a_symbol + 5;
	ret = sprint_symbol(buffer, address);
	printk("ret    :%d\n", ret );
	printk("buffer :%s\n", buffer );
		
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit sprint_symbol_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(sprint_symbol_init);
module_exit(sprint_symbol_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

