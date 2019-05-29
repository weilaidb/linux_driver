
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/resource.h>

#include "../include/types.h"


_syscall4(int, audit, u8, type, u8 *, buf, u16, len, u8, reset);

#define AUDIT_BUF_SIZE 100*sizeof(struct syscall_buf)
    
int main(int argc, char *argv[])
{
    u8 col_buf[AUDIT_BUF_SIZE];
   // u32 buf_count = 0;
    unsigned char reset =1;//not use
    int num;
    /* get the data from the kernel one collection buffer at a time */
    while (1) {
        //printf(" begin to audit !\n");
        num = audit(0, col_buf, AUDIT_BUF_SIZE, reset);
        //num = audit(0, col_buf, AUDIT_BUF_SIZE, reset);
       // printf(flag,"%d");
        u8 j=0;
        int i;
        for (i=0;i<num;i++)
        {   
        printf("-----------------------------------------------\n");
        printf("serial: %d\n", ((struct syscall_buf*)col_buf)[i].serial); 
        printf("ts_sec: %d\n", ((struct syscall_buf*)col_buf)[i].ts_sec);
        printf("ts_micro: %d\n", ((struct syscall_buf*)col_buf)[i].ts_micro);
        printf("syscall: %d\n", ((struct syscall_buf*)col_buf)[i].syscall);
        printf("status: %d\n", ((struct syscall_buf*)col_buf)[i].status);
        printf("pid: %d\n", ((struct syscall_buf*)col_buf)[i].pid); 
        printf("uid: %d\n", ((struct syscall_buf*)col_buf)[i].uid);
        printf("euid: %d\n", ((struct syscall_buf*)col_buf)[i].euid);
        printf("device: %d\n", ((struct syscall_buf*)col_buf)[i].device);
        printf("comm: %s\n", ((struct syscall_buf*)col_buf)[i].comm);
      
        }

        }
   
    return 1;
}

