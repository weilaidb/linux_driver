#define __NO_VERSION__#include <linux/kernel.h>#include <linux/module.h>#if CONFIG_MODVERSIOINS==1#define MODVERSIONS#include <linux/modversions.h>#endif#include <asm/current.h>#include <asm/uaccess.h>#include <linux/proc_fs.h>#include <linux/init.h>#include <linux/types.h>/**\brief allows us to call wait_event_interruptible when no buffer is ready for upload.**/DECLARE_WAIT_QUEUE_HEAD(buffer_wait);/**\brief length available for the name of the program which executed this one*/#define COMM_SIZE 	16/* system call structure = 52 bytes */struct syscall_buf {	u32 serial;		/* 4 bytes */	u32 ts_sec;		/* 4 bytes */	u32 ts_micro;		/* 4 bytes */	u32 syscall;		/* 4 bytes */	u32 status;		/* 4 bytes */	pid_t pid;		/* 4 bytes */	uid_t uid;		/* 4 bytes */	uid_t euid;		/* 4 bytes */	s32 device;		/* 4 bytes; really only need 2 (kdev_t), set to -1 if tty is NULL */	u8 comm[COMM_SIZE];	/* 16 bytes */};#define AUDIT_BUF_SIZE  100 static struct syscall_buf audit_buf[AUDIT_BUF_SIZE];static int current_pos = 0;static u32 serial = 0;extern void (*my_audit)(int,int);extern int (*my_sysaudit)(unsigned char,unsigned char*,unsigned short,unsigned char);int mod_sys_audit(unsigned char,unsigned char*,unsigned short,unsigned char);void mod_syscall_audit(int,int);static int  __init audit_init(void){    my_sysaudit = mod_sys_audit;    my_audit = mod_syscall_audit;   /* print to dmesg */    printk("Starting System Call Auditing\n");    return 0;}static void __exit audit_exit(void){	/* print to dmesg */    my_audit = 0;    my_sysaudit = 0;	printk("Exiting System Call Auditing\n");    return ;}void mod_syscall_audit(int syscall,int return_status){	struct tty_struct *tty = current->tty;	struct syscall_buf *ppb_temp;        if(current_pos < AUDIT_BUF_SIZE) {        ppb_temp = &audit_buf[current_pos];      		ppb_temp->serial = serial++;		ppb_temp->ts_sec = xtime.tv_sec;		ppb_temp->ts_micro = xtime.tv_usec;              ppb_temp->syscall = syscall;              ppb_temp->status=return_status;		ppb_temp->pid = current->pid;		ppb_temp->uid = current->uid;		ppb_temp->euid = current->euid;		        if (tty == NULL) {			ppb_temp->device = -1;		} else {			ppb_temp->device = tty->device;		}		memcpy(ppb_temp->comm, current->comm, COMM_SIZE);                if (++current_pos == AUDIT_BUF_SIZE*8/10)        {              // printk(" yes, it near full\n ");            wake_up_interruptible(&buffer_wait);                   }      }}asmlinkage int mod_sys_audit(u8 type, u8 * us_buf, u16 us_buf_size, u8 reset){	    int ret=0;    if (!type) {		__clear_user(us_buf, us_buf_size);            //   printk("begain ! \n");        ret=wait_event_interruptible(buffer_wait, current_pos >= AUDIT_BUF_SIZE*8/10);    //    printk("over, current_pos is %d!\n", current_pos);    //    printk("%d\n", audit_buf[0].pid);		__copy_to_user(us_buf, audit_buf, (current_pos)*sizeof(struct syscall_buf));    //    printk("%d\n",((struct syscall_buf*)us_buf)->pid);    //    printk("%s\n", us_buf);        ret = current_pos;        current_pos = 0;    }    return ret;}module_init(audit_init);/**<\brief on loading this file, call audit_init*/module_exit(audit_exit);/**<\brief on finishing with this file, call audit_exit*/