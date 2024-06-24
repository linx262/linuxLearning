#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DTSLED_CNT        1                /* 设备号个数 */
#define DTSLED_NAME        "dtsled"        /* 设备名 */

/* dtsled 设备结构体 */
struct dtsled_dev {
    dev_t devid;                /* 设备号 */
    struct cdev cdev;            /* 字符设备 */
    struct class *class;        /* 类 */
    struct device *device;        /* 设备 */
    int major;                    /* 主设备号 */
    int minor;                    /* 次设备号 */
};

static int dtsled_open( struct inode *inode, struct file *filp) 
{
    filp->private_data = &dtsled;
    return 0;
}

static int dtsled_release( struct inode *inode, struct file *filp) 
{
    struct dtsled_dev *dev = (struct dtsled_dev*)filp->private_data;
    return 0;
}

static ssize_t dtsled_write( struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    struct dtsled_dev *dev = (struct dtsled_dev*)filp->private_data;
    return 0;
}

/* dtsled 字符设备操作集 */
struct const struct file_operations dtsled_fops = {
    .owner = THIS_MODULEL,
    .write = dtsled_write,
    .open = dtsled_open,
    .release = dtsled_release, 
};

struct dtsled_dev dtsled; /* LED设备 */

/* 入口 */
static int __init dtsled_init(void)
{
    int ret = 0;
    
    /* 注册字符设备 */
    
    /* 1.申请设备号 */
    dtsled.major = 0;    /* 设备号由内核分配 */
    if ( dtsled.major ) { /* 定义了主设备号 */
    
        dtsled.devid = MKDEV( dtsled.major, 0);
        ret = register_chrdev_region( dtsled.devid, DTSLED_CNT, DETLED_NAME);
        
    } else { /* 没有给定设备号 */
        
        ret = alloc_chrdev_region( &dtsled.devid, 0, DTSLED_CNT, DETLED_NAME);
        dtsled.major = MAJOR( dtsled.devid );
        dtsled.minor = MINOR( dtsled.devid );
        
    }
    
    if ( ret < 0 ) {
        goto fail_devid;
    }
    
    /* 2.添加字符设备 */
    dtsled.cdev.owner = THIS_MODULEL;
    cdev_init( &dtsled.cdev, &dtsled_fops );
    ret = cdev_add( &dtsled.cdev, dtsled.devid, DTSLED_CNT);
    if ( ret < 0 ) {
        goto fail_cdev;
    }
    
    /* 3.自动创建设备节点 */
    dtsled.class = class_create( THIS_MODULEL, DETLED_NAME );
    if ( IS_ERR( dtsled.class ) ) {
        ret = PTR_ERR( dtsled.class );
        goto fail_class;
    }
    
    dtsled.device = device_create( dtsled.class, NULL, dtsled.devid, NULL, DTSLED_NAME);
    if ( IS_ERR( dtsled.device ) ) {
        ret = PTR_ERR( dtsled.device );
        goto fail_device;
    }
    
    return 0;

fail_device:
    class_destroy( dtsled.class);

fail_class:
    cdev_del( &dtsled.cdev );

fail_cdev:
    unregister_chrdev_region( dtsled.devid, DTSLED_CNT);/* 释放设备号 */
    
fail_devid:
    return ret;
    

}

/* 出口 */
static void __exit dtsled_exit(void)
{
    /* 删除字符设备 */
    cdev_del( &dtsled.cdev );
    
    /* 释放设备号 */
    unregister_chrdev_region( dtsled.devid, DTSLED_CNT);
    
    /* 摧毁设备 */
    device_destroy( dtsled.class, dtsled.devid);
    
    /* 摧毁类 */
    class_destroy( dtsled.class );
    
}

/* 注册驱动和卸载驱动 */
module_init(dtsled_init);
module_exit(dtsled_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linx");
