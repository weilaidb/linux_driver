#include <linux/module.h>
#include <linux/init.h>

// cat /proc/kallsyms  查看内核的符号表

//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init test_module_init(void);
static void __exit test_module_exit(void);

int symbol_A(void)
{
	return 1;
}
EXPORT_SYMBOL(symbol_A);

int symbol_B(void)
{
	return 0;
}
EXPORT_SYMBOL(symbol_B);

static int symbol_1 = 1;
int symbol_2 = 2;
int symbol_3 = 3;
EXPORT_SYMBOL(symbol_3);

static int __init test_module_init(void)
{
	
	printk("<0>hello world\n");

    return 0;
}
//模块卸载函数
static void __exit test_module_exit(void)
{
    printk("<0>module[%s] exit ok!\n", __FUNCTION__);
}
//模块注册
module_init(test_module_init);
module_exit(test_module_exit);
//可选
MODULE_AUTHOR("wwww");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

