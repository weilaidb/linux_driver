#include <linux/module.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/sched.h>
#include <linux/delay.h>

//定义宏
#define PNUM 5
#define CNUM 5
#define BUF_NUM 10

//必选
//模块许可声明
MODULE_LICENSE("GPL");
int productor_thread(void *p)
{
	int i=*(int *)p;
	int p_num=PNUM;

	while(p_num)
	{
		if((s1.count)<=0)
		{
			printk("[producer %d,%d]:I will be waiting for producting..\n",i,s1.count);
		}
		down(&s1);
		down(&mutex);
		buf[in]=i*100+(PNUM-p_num+1);
		printk("[producer %d,%d]:I producted a goods \"%d\" to buf[%d]..\n",i,s1.count,buf[in],in);
		in=(in+1)%BUF_NUM;
		up(&mutex);
		up(&s2);
		p_num--;
	}
	return 0;
}

int consumer_thread(void *p)
{
	int i=*(int *)p;
	int goods;

	while(cflag!=CNUM)
	{
		if((s2.count)<=0)
		{
			printk("[consumer %d,%d]:I will be waiting for goods..\n",i,s2.count);
		}
		down(&s2);
		down(&mutex);
		goods=buf[out];
		printk("[consumer %d,%d]:I consumed a goods \"%d\" from buf[%d]..\n",i,s2.count,goods,(out%BUF_NUM));
		out=(out+1)%BUF_NUM;
		up(&mutex);
		up(&s1);
		cflag++;
	}
	return 0;
}

//模块加载函数
static int __init np_nc_init(void)
{
	int i;

	printk("np_nc module is working..\n");
	in=out=0;
	cflag=0;
	init_MUTEX(&mutex);
	sema_init(&s1,BUF_NUM);
	sema_init(&s2,0);

	for(i=0;i< N;i++)
	{
		index[i]=i+1;
		kernel_thread(productor_thread,&(index[i]),CLONE_KERNEL);
		kernel_thread(consumer_thread,&(index[i]),CLONE_KERNEL);
	}

	return 0;
}
//模块卸载函数
static void np_nc_exit(void)
{
	printk(KERN_ALERT "np_nc  goodbye,kernel\n");
}
//模块注册
module_init(np_nc_init);
module_exit(np_nc_exit);
//可选
MODULE_AUTHOR("edsionte Wu");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");

