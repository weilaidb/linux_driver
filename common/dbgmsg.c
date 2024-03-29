/*=======================================================
* 文件说明
* 描    述:调试打印接口
* 文件名称:dbgmsg.c
*========================================================*/

/*=======================================================
* 标准头文件、头文件
*========================================================*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>  
#include "include/publicdef.h"
#include "include/publicbasic.h"
#include "include/dbgmsg.h"

//模块许可声明
MODULE_LICENSE("GPL");
/*=======================================================
* 宏定义
*========================================================*/

/*=======================================================
* 结构体，数据类型定义
*========================================================*/

/*=======================================================
* 全局变量，局部变量定义
*========================================================*/


/*=======================================================
* 函数声明
*========================================================*/


/*=======================================================
* 函数、类实现
*========================================================*/
WORD32 DbgPrintMsg(VOID *pMsg, WORD32 dwLen)
{
	WORD32 dwRet = 0;
	WORD32 dwCur = 0;
	WORD32 dwNum = 0;
	CHAR ucBuf[64];
	memset(ucBuf, 0, sizeof(ucBuf));
	printk("--------------------- begin\n");
	TLVLOOPS(dwLp, dwLen)
	{
		if(0 == dwLp % 16)
		{
			// printk("---------------------\n");
			printk(" %s\n", ucBuf);
			memset(ucBuf, 0, sizeof(ucBuf));
			dwCur = 0;
		}
		else
		{
			dwNum = snprintf(ucBuf + dwCur, 3, "%02x", *(BYTE *)(pMsg + dwLp));
			// printk("dwNum :%u\n", dwNum);
			// printk("dwCur :%u\n", dwCur);
			dwCur += dwNum;
		}
	}
	printk("\n");
	printk("--------------------- end\n");
	
	return 0;
}

EXPORT_SYMBOL(DbgPrintMsg);



