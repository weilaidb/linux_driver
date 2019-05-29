

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/types.h>



void (*my_audit)(int, int) = 0;

asmlinkage void syscall_audit(int syscall,int return_status)
{       
        if(my_audit)
        return = (*my_audit)(syscall, return_status);
	return = -1;
}

int (*my_sysaudit)(u8,u8*,u16,u8)=0;
asmlinkage int sys_audit(u8 type, u8 * us_buf, u16 us_buf_size, u8 reset){
    if(my_sysaudit)
    (*my_sysaudit)(type,us_buf,us_buf_size,reset);
}

