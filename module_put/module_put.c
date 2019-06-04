#include <linux/module.h>
#include <linux/init.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init module_put_init(void);
static void __exit module_put_exit(void);

static int __init module_put_init(void)
{
	const char *name = "test_module";
	struct module *fmodule = find_module(name);
	
	if(NULL != fmodule)
	{
		printk("before callling  module_put,\n");
		printk("refs of %s is: %d\n", name, module_refcount(fmodule));
		module_put(fmodule);
		
		printk("after callling  module_put,\n");
		printk("refs of %s is: %d\n", name, module_refcount(fmodule));
		
	}
	else
	{
		printk("find %s failed!\n", name );
	}
    return 0;
}
//模块卸载函数
static void __exit module_put_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(module_put_init);
module_exit(module_put_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

