#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/uaccess.h>


#define kthread_run(threadfn, data, namefmt, ...)                          \
({                                                                         \
        struct task_struct *__k                                            \
                = kthread_create(threadfn, data, namefmt, ## __VA_ARGS__); \
        if (!IS_ERR(__k))                                                  \
                wake_up_process(__k);                                      \
        __k;                                                               \
})


static int   dmatest_work (void *data)
{
    allow_signal(SIGTERM);
    current->state = TASK_INTERRUPTIBLE;
 
	printk("New kernel thread run\n");
    
	return 0;
}
 
static int __init dmatest_init(void)
{
    /* Schedule the test thread */
    kthread_run (dmatest_work, NULL, "dmatest");
 
    return 0;
}
 
static void __exit dmatest_exit(void)
{
    return;
}
 
MODULE_LICENSE("Dual BSD/GPL");
module_init(dmatest_init);
module_exit(dmatest_exit);