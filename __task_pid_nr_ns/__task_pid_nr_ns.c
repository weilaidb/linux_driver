#include <linux/module.h>
#include <linux/init.h>
#include <linux/kallsyms.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init try_module_get_init(void);
static void __exit try_module_get_exit(void);

static int __init try_module_get_init(void)
{
	int ret;
	const char *name;
	struct module *fmodule = NULL;
	name = "test_module";
	fmodule = find_module(name);
	if(NULL != NULL)
	{
		printk("before calling try_module_get,\n");
		printk("refs of %s is: %d\n", name, module_refcount(fmodule));
		ret = try_module_get(fmodule);
		printk("after calling try_module_get,\n");
		printk("ret=%d\n", ret);
		printk("refs of %s is: %d\n", name, module_refcount(fmodule));
	}
	else
	{
		printk("find %s failed!\n", name);
	}
	
	printk("module_refcount(THIS_MODULE):%d\n", module_refcount(THIS_MODULE) );
    return 0;
}
//模块卸载函数
static void __exit try_module_get_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(try_module_get_init);
module_exit(try_module_get_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

