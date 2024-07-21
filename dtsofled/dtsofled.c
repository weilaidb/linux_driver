#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>


//必选
//模块许可声明
MODULE_LICENSE("GPL");
static int __init dtsofled_init(void);
static void __exit dtsofled_exit(void);

static int __init dtsofled_init(void)
{
    int ret = 0;

    struct device_node *bl_nd = NULL;
    struct property *comppro = NULL;
    const char *str = NULL;
    u32 def_value = 0;
    u32 elemsize = 0;
    u32 *brival = NULL;
    /* 1.找到backlight节点，路径是/backlight */
    bl_nd = of_find_node_by_path("/backlight");
    if(bl_nd == NULL) {
        printk(KERN_ALERT "dtsofled: can't find backlight node!\n");
        ret = -EINVAL;
        goto fail_findnd;
    }
    printk(KERN_ALERT "dtsofled: find backlight node!\n");
    /*2. 获取属性 */
    comppro = of_find_property(bl_nd, "compatible", NULL);
    if(comppro == NULL) {
        printk(KERN_ALERT "dtsofled: can't find compatible property!\n");
        ret = -EINVAL;
        goto fail_property;
    }
    printk(KERN_ALERT "dtsofled: find compatible property!\n");
    printk(KERN_ALERT "dtsofled: %s\n", (char *)comppro->value);
    ret = of_property_read_string(bl_nd, "status", &str);
    if(ret != 0) {
        printk(KERN_ALERT "dtsofled: can't find status property!\n");
        goto fail_rs;
    }
    else {
        printk(KERN_ALERT "status: %s\n", str);
    }
    /*3、读取数字属性值*/
    ret = of_property_read_u32(bl_nd, "default-brightness_level", &def_value);
    if(ret != 0) {
        goto fail_read32;
    }
    else {
        printk(KERN_ALERT "default-brightness_level: %u\n", def_value);
    }

    /*4、获取数组类型的属性*/
    elemsize = of_property_count_elems_of_size(bl_nd, "brightness-levels", sizeof(u32));
    if(elemsize < 0) {
        ret = -EINVAL;
        goto fail_readele;
    }
    else {
        printk(KERN_ALERT "dtsofled: brightness-levels size: %d\n", elemsize);
    }
    /* 申请内存*/
    brival = kmalloc(elemsize * sizeof(u32), GFP_KERNEL);
    if(brival == NULL) {
        printk(KERN_ALERT "dtsofled: kmalloc failed!\n");
        ret = -ENOMEM;
        goto fail_mem;
    }

    /* 5、读取数组类型的属性 */
    ret = of_property_read_u32_array(bl_nd, "brightness-levels", brival, elemsize);
    if(ret != 0) {
        goto fail_read32array;
    }
    else {
        int i = 0;
        for(i = 0; i < elemsize; i++) {
            printk(KERN_ALERT "dtsofled: brightness-levels[%d]: %u\n", i, brival[i]);
        }
    }

fail_read32array:
    kfree(brival);

    return 0;


fail_readele:
fail_mem:
fail_property:
fail_findnd:
fail_rs:
fail_read32:

    return ret;
}
//模块卸载函数
static void __exit dtsofled_exit(void)
{
    printk(KERN_ALERT "dtsofled exit ok!\n");
}




//模块注册
module_init(dtsofled_init);
module_exit(dtsofled_exit);
//可选
MODULE_AUTHOR("dtsofled");
MODULE_DESCRIPTION("dtsofled simple example!\n");
MODULE_ALIAS("dtsofled simplest example");

