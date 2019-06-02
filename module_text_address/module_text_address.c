#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include "../common/include/publicdef.h"
#include "../common/include/publicbasic.h"
#include "../common/include/dbgmsg.h"
#include "../common/dbgmsg.c"


extern WORD32 DbgPrintMsg(VOID *pMsg, WORD32 dwLen);
//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init __module_text_address_init(void);
static void __exit __module_text_address_exit(void);
//模块加载函数
int fun_test(void)
{
	return 0;
}

static int __init __module_text_address_init(void)
{
	struct module * ret;
	unsigned long addr  = (unsigned long)fun_test;
	/**
	** 调用__module_address()函数之前，必须禁止中断，以防止模块在执行操作期间被释放
	**/
	preempt_disable();
	ret = __module_text_address(addr);
	preempt_enable();
	
	printk("it's about fun_test\n");
	if(ret != NULL)
	{
		DbgPrintMsg((VOID *)ret, sizeof(*ret));
		printk("size struct module      :%d\n", sizeof(*ret));
		printk("ret->name               :%s\n", ret->name);
		printk("ret->state              :%d\n", ret->state);
		printk("ret->version            :%s\n", ret->version);
		printk("ret->srcversion         :%s\n", ret->srcversion);
		// printk("ret->core_size :%d\n", ret->core_size);
		printk("refs of %s is %d\n", ret->name, module_refcount(ret));
	}
	else
	{
		printk(KERN_ALERT "__module_address return NULL!\n");		
	}
	
	
    return 0;
}
//模块卸载函数
static void __exit __module_text_address_exit(void)
{
    printk(KERN_ALERT "module exit ok!\n");
}
//模块注册
module_init(__module_text_address_init);
module_exit(__module_text_address_exit);
//可选
MODULE_AUTHOR("edsionte Wu");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

