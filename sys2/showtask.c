/***************************************************************************
** 版权所有:   Copyright (c) 2020-2030
** 文件名称:  showtask.c
** 当前版本:  v1.0
** 作    者:
** 完成日期: 2025-02-08
** 修改记录:
** 修改日期:
** 版本号  :
** 修改人  :
** 修改内容:
***************************************************************************/

/*****************************头文件****************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/pid.h>
#include <linux/fs.h>

#include "showtask.h"


/*****************************宏定义****************************************/

/*****************************结构体或类型定义*******************************/

/*****************************全局变量****************************************/

/*****************************本地变量****************************************/

/*****************************函数或类声明****************************************/

/*****************************函数或类实现****************************************/

void showtasklist(const char *param)
{
    struct task_struct *task;
    int iTaskCount = 0;

    for_each_process(task)
    {
        printk(KERN_INFO "Process: %s (PID: %d)\n", task->comm, task->pid);
        iTaskCount++;
    }
    printk(KERN_INFO "task count:%d\n", iTaskCount);
}

// EXPORT_SYMBOL(showtasklist);

void print_task_info(const char *param)
{
    struct task_struct *task;
    int iTaskCount = 0;

    for_each_process(task)
    {
        pid_t pid = task->pid;
        pid_t tgid = task->tgid;
        int prio = task->prio;
        int static_prio = task->static_prio;
        int normal_prio = task->normal_prio;

        printk(KERN_INFO "Process: %s (PID: %d, TGID: %d, Prio: %d, Static Prio: %d, Normal Prio: %d)\n",
               task->comm, pid, tgid, prio, static_prio, normal_prio);
        iTaskCount++;
    }
    printk(KERN_INFO "task count:%d\n", iTaskCount);
}

// EXPORT_SYMBOL(print_task_info);

// 设置所有进程的优先级
static void set_all_task_nice(const char *param)
{
    int nice_value = 0;
    if ((sscanf(param, "%d", &nice_value) >= 1) && (nice_value >= -20 && nice_value <= 19))
    {
        struct task_struct *task;
        for_each_process(task)
        {
            if (task->pid != 0)
            { // 跳过 init 进程
                set_user_nice(task, nice_value);
                printk(KERN_INFO "Set nice value of process %s (PID: %d) to %d\n", task->comm, task->pid, nice_value);
            }
        }
    }
    else
    {
        printk(KERN_ERR "Invalid nice value\n");
    }
}

// 设置指定用户的进程的优先级
static void set_user_nice_by_uid(const char *param)
{
    int uid, nice_value;
    bool flag = false;

    if (sscanf(param, "%d %d", &uid, &nice_value) == 2)
    {
        struct task_struct *task;
        kuid_t kuid = make_kuid(current_user_ns(), uid);

        if (!uid_valid(kuid))
        {
            printk(KERN_ERR "Invalid UID: %d\n", uid);
            return;
        }

        if (nice_value < -20 || nice_value > 19)
        {
            printk(KERN_ERR "Invalid nice value: %d\n", nice_value);
            return;
        }

        for_each_process(task)
        {
            if (task->pid != 0 && uid_eq(task_uid(task), kuid))
            { // 跳过 init 进程
                set_user_nice(task, nice_value);
                printk(KERN_INFO "Set nice value of process %s (PID: %d, UID: %d) to %d\n", task->comm, task->pid, uid, nice_value);
                flag = true;
            }
        }
        if (!flag)
        {
            printk(KERN_ERR "No process found with UID: %d\n", uid);
        }
    }
    else
    {
        printk(KERN_ERR "Invalid input format. Expected: setniceuser <uid> <nice_value>\n");
    }
}
// 打印所有线程的函数
void showallthread(const char *param)
{
    struct task_struct *task, *thread;

    // 打印标题
    printk(KERN_INFO "%-6s%-10s%-30s%-10s%s\n", "PID", "TID", "Name", "Priority", "Policy");
    printk(KERN_INFO "--------------------------------------------------------------------------------\n");

    // 遍历所有进程
    for_each_process(task)
    {
        // 遍历当前进程的所有线程
        for_each_thread(task, thread)
        {
            // 获取线程的优先级和调度策略
            int prio = thread->prio;     // 线程的动态优先级
            int policy = thread->policy; // 线程的调度策略

            // 将调度策略转换为字符串
            const char *policy_str;
            switch (policy)
            {
            case SCHED_NORMAL:
                policy_str = "SCHED_NORMAL";
                break;
            case SCHED_FIFO:
                policy_str = "SCHED_FIFO";
                break;
            case SCHED_RR:
                policy_str = "SCHED_RR";
                break;
            case SCHED_BATCH:
                policy_str = "SCHED_BATCH";
                break;
            case SCHED_IDLE:
                policy_str = "SCHED_IDLE";
                break;
            case SCHED_DEADLINE:
                policy_str = "SCHED_DEADLINE";
                break;
            default:
                policy_str = "UNKNOWN";
            }

            // 打印线程信息
            printk(KERN_INFO "%-6d%-10d%-30s%-10d%s\n",
                   task_pid_nr(task),   // 进程 ID
                   task_pid_nr(thread), // 线程 ID
                   thread->comm,        // 线程名称
                   prio,                // 线程优先级
                   policy_str);         // 调度策略
        }
    }
}