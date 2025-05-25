#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h> // Include this header for cdev-related functionality
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include "platform.h"

#undef pr_fmt
//#define pr_fmt(fmt) fmt
#define pr_fmt(fmt) "%s:" fmt, __func__

#define MEM_SIZE_MAX_PCDEV1 1024
#define MEM_SIZE_MAX_PCDEV2 512
#define MEM_SIZE_MAX_PCDEV3 1024
#define MEM_SIZE_MAX_PCDEV4 512

#define NO_OF_DEVICES 4

/*psuedo devices memory*/
char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];

/*Device private data structure*/
struct pcdev_private_data{
    struct pcdev_platform_data pdata;
    char *buffer;
    dev_t dev_num;
    struct cdev cdev;

};
/*Driver private data structure*/
struct pcdrv_private_data
{
    int total_devices;
    dev_t device_num_base;
    struct class *class_pcd;
    struct device *device_pcd;
};
struct pcdrv_private_data  pcdrv_data;


#define READ_ONLY 0x1
#define WRITE_ONLY 0x10
#define READ_WRITE 0x11

// Function prototypes
loff_t pcd_lseek(struct file *filp, loff_t off, int whence);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *filp);
int check_permission(int access_mode, int dev_permission);
int pcd_platform_driver_probe(struct platform_device *pdev);
void pcd_platform_driver_remove(struct platform_device *pdev);

#define MAX_DEVICES 4
static int __init pcd_platform_driver_nit(void)
{
    int ret;
    /* 1. Dynamically allocate a device number for MAX_DEVICES*/
    ret = alloc_chrdev_region(&pcdrv_data.device_num_base, 0, MAX_DEVICES, "pcdevs");
    if(ret < 0)
    {
        pr_err("alloc chrdev failed\n");
        return ret;
    }
    /* 2. Create Device class under /sys/class */
    pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
    if(IS_ERR(pcdrv_data.class_pcd))
    {
        pr_err("class creation failed\n");
        ret = PTR_ERR(pcdrv_data.class_pcd);
        unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
        return ret; 
    }

    /* 3. Register a platform driver*/
    platform_driver_register(&pcd_platform_driver);
    pr_info("pcd platform driver loaded\n");
    return 0;

}
loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
 
    return 0;
}
ssize_t pcd_read (struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
 
    /*Return number of bytes successfully read*/
    return 0;
}
ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    return -ENOMEM;
}



int check_permission(int access_mode, int dev_permission)
{
    pr_info("Device permission = %d\n", dev_permission);
    pr_info("User permission = %d\n", access_mode);

    if(dev_permission == READ_WRITE)
    {
        pr_info("Read and write permission granted\n");
        return 0;
    }
    else if((dev_permission == READ_ONLY) && ((access_mode & FMODE_READ) && !(access_mode & FMODE_WRITE)))
    {
        pr_err("Read and write permission granted\n");
        return 0;
    }
    else if((dev_permission == WRITE_ONLY) && ((access_mode & FMODE_WRITE) && !(access_mode & FMODE_READ)))
    {
        pr_err("Read and write permission granted\n");
        return 0;
    }
    
    return -EPERM;
}
int pcd_open (struct inode *inode, struct file *filp)
{
    return 0;
}
int pcd_release (struct inode *inode, struct file *filp)
{
    pr_info("release called\n");
    return 0;
}

//struct device *device_create(struct class *class, struct device *parent, dev_t devt, void *drvdata, const char *fmt, ...);
//file operations of driver
struct file_operations pcd_fops = {
    .open = pcd_open,
    .release = pcd_release,
    .read = pcd_read,
    .write = pcd_write,
    .llseek = pcd_lseek
};
 
/*This function gets called when matched platform device is found*/
int pcd_platform_driver_probe(struct platform_device *pdev)
{
    /* 1. Get the platform data*/

    /* 2. Dynamically allocate memory for the device private data*/

    /* 3. Dynamicallyn allocate memory for the device buffer using size
    information from the platform data */

    /* 4. Get the device number */

    /* 5. cdev init and cdev add*/

    /* 6. Create device file for detected platform device*/

    /* 7. Error handeling */

    pr_info("matched platform device is deteced\n");
    return 0;
}
/*remove function gets called when the device is removed from system*/
void pcd_platform_driver_remove(struct platform_device *pdev)
{
    pr_info("pcd platform driver removed\n");

}
struct platform_driver pcd_platform_driver = {
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .driver = {
        .name = "psuedo_char_device",
        .owner = THIS_MODULE,
    },
};


static void __exit pcd_driver_cleanup(void)
{ 
    /* 1. Unregister the platform driver */
    platform_driver_unregister(&pcd_platform_driver);

    /* 2. Class destroy */
    class_destroy(pcdrv_data.class_pcd);

    /* 3. unregister device numbers for MAX_DEVICES*/
    unregister_chrdev_region(&pcdrv_data.device_num_base, MAX_DEVICES);
    pr_info("pcd platform driver unloaded\n");
}

module_init(pcd_driver_nit);
module_exit(pcd_driver_cleanup);


MODULE_DESCRIPTION("Pseudo character driver");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_ALIAS("pcd");


