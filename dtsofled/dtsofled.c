#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/io.h>
//#include <linux/io_address.h>
// #include <linux/io_irq.h>
#include <linux/cdev.h>
#include <linux/fs.h>


#define DTSLED_CNT 1
#define DTSLED_NAME "dtsled"


#define LEDOFF 0
#define LEDON 1


/*地址映射后的虚拟指针地址*/
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_I003;
static void __iomem *SW_PAD_GPIO1_I003;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;




static int __init dtsofled_init(void);
static void __exit dtsofled_exit(void);

struct dtsled_dev {
    dev_t devid; /*设备号*/
    struct cdev cdev;/*字符设备*/
    struct device *device; /*设备结构体*/
    struct class *class; /*类指针*/
    int major; /*主设备号*/
    int minor; /*次设备号*/
    struct device_node *nd; /*dts节点*/
}; /*定义dtsled设备结构体*/

struct dtsled_dev dtsled; /*led设备*/

/* LED灯打开/关闭 */
static void led_switch(u8 sta)
{
    u32 val = 0;

    if(sta == LEDON) {
        val = readl(GPIO1_DR);
        val &= ~(1 << 3); /* bit3清零，打开LED灯*/
        writel(val, GPIO1_DR);
    }else if(sta == LEDOFF) {
        val = readl(GPIO1_DR);
        val |= (1 << 3); /*bit3置1，关闭LED灯*/
        writel(val, GPIO1_DR);
    }
}

static int dtsled_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &dtsled;
    return 0;
}

static int dtsled_release(struct inode *inode, struct file *filp)
{
    struct dtsled_dev *dev = (struct dtsled_dev *)filp->private_data;
    return 0;
}

static ssize_t dtsled_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    struct dtsled_dev *dev = (struct dtsled_dev *)filp->private_data;
    return 0;
}

static ssize_t dtsled_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    struct dtsled_dev *dev = (struct dtsled_dev *)filp->private_data;
    int retvalue;
    unsigned char databuf[1];
    retvalue = copy_from_user(databuf, buf, cnt);
    if(retvalue < 0) {
        return -EFAULT;
    }

    led_switch(buf[0]);

    return 0;
}


/*dtsled 字符设备操作集*/
static const struct file_operations dtsled_fops = {
    .owner = THIS_MODULE,
    .open = dtsled_open,
    .release = dtsled_release,
    .write = dtsled_write,
    .read = dtsled_read,

};

static int __init dtsofled_init(void)
{
    int ret = 0;
    const char *str;
    u32 regdata[10];
    int i;

    /*注册字符设备*/
    dtsled.major = 0; /* 设备号*/
    if(dtsled.major) {
        dtsled.devid = MKDEV(dtsled.major, 0);
        ret = register_chrdev_region(dtsled.devid, DTSLED_CNT, DTSLED_NAME);
    } else {
        ret = alloc_chrdev_region(&dtsled.devid, 0,DTSLED_CNT, DTSLED_NAME);
        dtsled.major = MAJOR(dtsled.devid);
        dtsled.minor = MINOR(dtsled.devid);
    }
    if(ret < 0) {
        goto fail_devid;
    }

    /*2.添加字符设备*/
    dtsled.cdev.owner = THIS_MODULE;
    cdev_init(&dtsled.cdev, &dtsled_fops);
    ret = cdev_add(&dtsled.cdev, dtsled.devid, DTSLED_CNT);
    if(ret < 0) {
        goto fail_cdev;
    }
    /*3、自动创建设备节点*/
    dtsled.class = class_create(THIS_MODULE, DTSLED_NAME);
    if(IS_ERR(dtsled.class)) {
        ret = PTR_ERR(dtsled.class);
        goto fail_class;
    }

    dtsled.device = device_create(dtsled.class, NULL, dtsled.devid, NULL, DTSLED_NAME);
    if(IS_ERR(dtsled.device)) {
        ret = PTR_ERR(dtsled.device);
        goto fail_device;
    }

/*获取设备树属性内容*/
    dtsled.nd = of_find_node_by_path("/alphaled");
    if(!dtsled.nd) {
        printk(KERN_ALERT "dts node not found!\n");
        goto fail_findnd;
    }
    ret = of_property_read_string(dtsled.nd, "status", &str);
    if(ret < 0) {
        printk(KERN_ALERT "dts node not found!\n");
        goto fail_rs;
    }
    printk(KERN_ALERT "status: %s\n", str);

    ret = of_property_read_string(dtsled.nd, "compatible", &str);
    if(ret < 0) {
        printk(KERN_ALERT "dts node not found!\n");
        goto fail_rs;
    }
    printk(KERN_ALERT "compatible: %s\n", str);

    ret = of_property_read_u32_array(dtsled.nd, "reg", regdata, sizeof(regdata));
    if(ret < 0) {
        printk(KERN_ALERT "dts node not found!\n");
        goto fail_rs;
    }
    printk("reg data\n");
    for(i = 0; i < 10; i++) {
        printk(KERN_ALERT "reg[%d]: %x\n", i, regdata[i]);
    }
    
    /*LED初始化 */
    IMX6U_CCM_CCGR1=ioremap(regdata[0], regdata[1]);
    SW_MUX_GPIO1_I003 = ioremap(regdata[2], regdata[3]);
    SW_PAD_GPIO1_I003 = ioremap(regdata[4], regdata[5]);
    GPIO1_DR=ioremap(regdata[6], regdata[7]);
    GPIO1_GDIR=ioremap(regdata[8],regdata[9]);

    return 0;


fail_rs:
fail_findnd:
    device_destroy(dtsled.class, dtsled.devid);
fail_device:
    class_destroy(dtsled.class);
fail_class:
    cdev_del(&dtsled.cdev);
fail_cdev:
    unregister_chrdev_region(dtsled.devid, DTSLED_CNT);
fail_devid:
    return ret;
}
//模块卸载函数
static void __exit dtsofled_exit(void)
{
    u32 val = 0;
    val = readl(GPIO1_DR);
    val |= (1 << 3); /* bit3置1，关闭LED*/
    writel(val, GPIO1_DR);




    /*取消地址映射*/
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_I003 );
    iounmap(SW_PAD_GPIO1_I003 );
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);

    /* 删除字符设备*/
    cdev_del(&dtsled.cdev);
    unregister_chrdev_region(dtsled.devid, DTSLED_CNT);
    device_destroy(dtsled.class, dtsled.devid);
    class_destroy(dtsled.class);
    printk(KERN_ALERT "dtsofled exit ok!\n");
}




//模块注册
module_init(dtsofled_init);
module_exit(dtsofled_exit);
//必选
//模块许可声明
MODULE_LICENSE("GPL");
//可选
MODULE_AUTHOR("dtsofled");
MODULE_DESCRIPTION("dtsofled simple example!\n");
MODULE_ALIAS("dtsofled simplest example");

