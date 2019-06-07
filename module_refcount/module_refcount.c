#include <linux/module.h>
#include <linux/init.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init module_refcount_init(void);
static void __exit module_refcount_exit(void);

static int __init module_refcount_init(void)
{
	const char *name = "test_module";
	struct module *fmodule = find_module(name);
	
	if(NULL != fmodule)
	{
		printk("fmodule->name:%s\n", fmodule->name);
		printk("module_refcount(fmodule):%d\n", module_refcount(fmodule));
	}
	else
	{
		printk("find %s failed!\n", name );
	}

	printk("THIS_MODULE->name:%s\n", THIS_MODULE->name );
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit module_refcount_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(module_refcount_init);
module_exit(module_refcount_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

