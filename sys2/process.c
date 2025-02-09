/***************************************************************************
** 版权所有:   Copyright (c) 2020-2030
** 文件名称:  process.c
** 当前版本:  v1.0
** 作    者:
** 完成日期: 2025-02-09
** 修改记录:
** 修改日期:
** 版本号  :
** 修改人  :
** 修改内容:
***************************************************************************/

/*****************************头文件****************************************/

/*****************************宏定义****************************************/

/*****************************结构体或类型定义*******************************/

/*****************************全局变量****************************************/

/*****************************本地变量****************************************/

/*****************************函数或类声明****************************************/

/*****************************函数或类实现****************************************/
#ifdef OPEN_SHOWPROCESSFD

#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sched/prio.h>
#include <linux/pid.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <net/rtnetlink.h>
#include <net/net_namespace.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <net/fib_rules.h>
#include <net/ip.h>
#include <net/route.h>
#include <linux/seq_file.h>
#include <linux/net.h>
#include <net/sock.h>
#include <linux/fs.h>
#include <linux/fdtable.h>

// 显示所有进程的文件描述符信息
int showprocessfd_in(const char *param)
{
    struct task_struct *task;
    struct files_struct *files;
    struct file *file;
    int fd;
    int total_fd_count = 0; // 总文件描述符计数器

    printk(KERN_INFO "All open file descriptors:\n");
    printk(KERN_INFO "%-8s%-8s%-10s%-15s%-15s\n", "PID", "FD", "Type", "RecvBufSize", "SendBufSize");

    // 遍历所有进程
    for_each_process(task)
    {
        files = task->files;
        if (!files)
            continue;

        int process_fd_count = 0; // 每个进程的文件描述符计数器

        // 遍历进程的文件描述符
        rcu_read_lock();
        struct fdtable *fdt = files_fdtable(files);
        for (fd = 0; fd < fdt->max_fds; fd++)
        {
            file = fdt->fd[fd];
            if (!file)
                continue;

            process_fd_count++; // 增加进程的文件描述符计数
            total_fd_count++;   // 增加总文件描述符计数

            // 获取文件描述符的类型和大小
            char type[10];
            // if (file->f_op && file->f_op->owner == THIS_MODULE) // 替代方法：检查文件操作结构体的所有者
            // {
            //     strcpy(type, "SOCKET");
            // }
            if (S_ISSOCK(file_inode(file)->i_mode))
            {
                strcpy(type, "SOCKET");
            }
            else if (S_ISREG(file_inode(file)->i_mode))
            {
                strcpy(type, "REGULAR");
            }
            else if (S_ISDIR(file_inode(file)->i_mode))
            {
                strcpy(type, "DIRECTORY");
            }
            else if (S_ISCHR(file_inode(file)->i_mode))
            {
                strcpy(type, "CHARACTER");
            }
            else if (S_ISBLK(file_inode(file)->i_mode))
            {
                strcpy(type, "BLOCK");
            }
            else if (S_ISFIFO(file_inode(file)->i_mode))
            {
                strcpy(type, "FIFO");
            }
            else if (S_ISLNK(file_inode(file)->i_mode))
            {
                strcpy(type, "SYMLINK");
            }
            else
            {
                strcpy(type, "UNKNOWN");
            }

            // 打印文件描述符信息
            if (strcmp(type, "SOCKET") == 0)
            {
                struct socket *sock = file->private_data;
                if (sock)
                {
                    struct sock *sk = sock->sk;
                    if (sk)
                    {
                        printk(KERN_INFO "%-8d%-8d%-10s%-15d%-15d\n",
                               task_pid_nr(task), fd, type,
                               sk->sk_rcvbuf, sk->sk_sndbuf);
                    }
                }
            }
            else
            {
                printk(KERN_INFO "%-8d%-8d%-10s%-15zu%-15zu\n",
                       task_pid_nr(task), fd, type,
                       file_inode(file)->i_size, file_inode(file)->i_size);
            }
        }
        rcu_read_unlock();

        // 打印每个进程的文件描述符数量
        printk(KERN_INFO "Process %d has %d file descriptors open.\n", task_pid_nr(task), process_fd_count);
    }

    // 打印总文件描述符数量
    printk(KERN_INFO "Total file descriptors open: %d\n", total_fd_count);

    return 0;
}

void showprocessfd(const char *param)
{
    showprocessfd_in(param);
}

#endif

#ifdef OPEN_SETSOCKETSIZE
// 修改指定进程的 socket 文件描述符的接收和发送缓冲区大小
int setsocketsize_in(const char *param)
{
    struct task_struct *task;
    struct files_struct *files;
    struct file *file;
    int pid, lfd, fd, new_rcv_size, new_snd_size;

    // 解析参数
    if (sscanf(param, "%d %d %d %d", &pid, &fd, &new_rcv_size, &new_snd_size) != 4)
    {
        printk(KERN_INFO "Invalid input format. Expected: setsocketsize <pid> <fd> <new_rcv_size> <new_snd_size>\n");
        return -EINVAL;
    }

    printk(KERN_INFO "Changing buffer sizes for PID %d, FD %d to RCV: %d, SND: %d\n", pid, fd, new_rcv_size, new_snd_size);

    // 遍历所有进程
    for_each_process(task)
    {
        if (task_pid_nr(task) != pid)
            continue;
        files = task->files;
        if (!files)
            continue;

        // 遍历进程的文件描述符
        rcu_read_lock();
        struct fdtable *fdt = files_fdtable(files);
        for (lfd = 0; lfd < fdt->max_fds; lfd++)
        {
            file = fdt->fd[lfd];
            if (!file)
                continue;
            if (lfd != fd)
            {
                continue;
            }
            if (file && S_ISSOCK(file_inode(file)->i_mode))
            {
                struct socket *sock = file->private_data;
                if (sock)
                {
                    struct sock *sk = sock->sk;
                    if (sk)
                    {
                        sk->sk_userlocks |= SOCK_RCVBUF_LOCK | SOCK_SNDBUF_LOCK;
                        sk->sk_rcvbuf = new_rcv_size;
                        sk->sk_sndbuf = new_snd_size;
                        printk(KERN_INFO "Buffer sizes for PID %d, FD %d changed to RCV: %d, SND: %d\n", pid, fd, new_rcv_size, new_snd_size);
                        return 0;
                    }
                }
            }
        }
        rcu_read_unlock();
    }

    printk(KERN_INFO "Socket with PID %d, FD %d not found\n", pid, fd);
    return -ENODEV;
}

void setsocketsize(const char *param)
{
    setsocketsize_in(param);
}

#endif