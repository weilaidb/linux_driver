#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>


//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init dtsof_init(void);
static void __exit dtsof_exit(void);

static int __init dtsof_init(void)
{
    int ret = 0;

    struct device_node *bl_nd = NULL;
    struct property *comppro = NULL;
    /* 1.找到backlight节点，路径是/backlight */
    bl_nd = of_find_node_by_path("/backlight");
    if(bl_nd == NULL) {
        printk(KERN_ALERT "dtsof: can't find backlight node!\n");
        ret = -EINVAL;
        goto fail_findnd;
    }
    printk(KERN_ALERT "dtsof: find backlight node!\n");
    /*2. 获取属性 */
    comppro = of_find_property(bl_nd, "compatible", NULL);
    if(comppro == NULL) {
        printk(KERN_ALERT "dtsof: can't find compatible property!\n");
        ret = -EINVAL;
        goto fail_property;
    }
    printk(KERN_ALERT "dtsof: find compatible property!\n");
    printk(KERN_ALERT "dtsof: %s\n", (char *)comppro->value);

    return 0;

fail_property:
fail_findnd:

    return ret;
}
//模块卸载函数
static void __exit dtsof_exit(void)
{
    printk(KERN_ALERT "dtsof exit ok!\n");
}
//模块注册
module_init(dtsof_init);
module_exit(dtsof_exit);
//可选
MODULE_AUTHOR("dtsof");
MODULE_DESCRIPTION("dtsof simple example!\n");
MODULE_ALIAS("dtsof simplest example");

