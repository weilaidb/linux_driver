/***************************************************************************
** 版权所有:   Copyright (c) 2020-2030
** 文件名称:  parse.c
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

/*****************************宏定义****************************************/

/*****************************结构体或类型定义*******************************/

/*****************************全局变量****************************************/

/*****************************本地变量****************************************/

/*****************************函数或类声明****************************************/

/*****************************函数或类实现****************************************/

// 解析用户输入的命令和参数
static void parse_command(const char *cmd)
{
    char *token, *cmd_copy;
    int param_count = 0;

    // 复制命令字符串，因为 strsep 会修改原字符串
    cmd_copy = kstrdup(cmd, GFP_KERNEL);
    if (!cmd_copy)
    {
        printk(KERN_ERR "Memory allocation failed\n");
        return;
    }

    // 使用 strsep 分割命令和参数
    token = strsep(&cmd_copy, " ");
    while (token)
    {
        param_count++;
        printk(KERN_INFO "Parameter %d: %s\n", param_count, token);
        token = strsep(&cmd_copy, " ");
    }

    kfree(cmd_copy);
}

// 打印内核符号表中的所有符号
static void showkernelsymbol(const char *param)
{
    // unsigned long addr;
    // const char *symbol;

    // printk(KERN_INFO "Kernel symbol table:\n");
    // for (addr = 0; addr < ULONG_MAX; addr += 4) {
    //     symbol = kallsyms_lookup(addr, NULL, NULL, NULL, NULL);
    //     if (symbol) {
    //         printk(KERN_INFO "[%016lx] %s\n", addr, symbol);
    //     }
    // }
}
