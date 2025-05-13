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
    unsigned size;
    const char *serial_number;
    int permission;

};
/*Driver private data structure*/
struct pcdrv_private_data
{
    int total_devices;
    dev_t device_num_base;
    dev_t device_number;
    struct class *pcd_class;
    struct device *device_pcd;
    struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};

#define READ_ONLY 0x1
#define WRITE_ONLY 0x10
#define READ_WRITE 0x11

struct pcdrv_private_data pcdrv_data = 
{
    .total_devices = NO_OF_DEVICES,
    .device_number = 0,
    .pcd_class = NULL,
    .device_pcd = NULL,
    .pcdev_data[0] = {
        .buffer = device_buffer_pcdev1,
        .size = MEM_SIZE_MAX_PCDEV1,
        .serial_number = "PCD1",
        .permission = READ_ONLY, /*Read Only*/
    },
    .pcdev_data[1] = {
        .buffer = device_buffer_pcdev2,
        .size = MEM_SIZE_MAX_PCDEV2,
        .serial_number = "PCD2",
        .permission = WRITE_ONLY, /*Write only*/
    },
    .pcdev_data[2] = {
        .buffer = device_buffer_pcdev3,
        .size = MEM_SIZE_MAX_PCDEV3,
        .serial_number = "PCD3",
        .permission = READ_WRITE, /*Read Write only*/
    },
    .pcdev_data[3] = {
        .buffer = device_buffer_pcdev4,
        .size = MEM_SIZE_MAX_PCDEV4,
        .serial_number = "PCD4",
        .permission = READ_WRITE, /*Read Write only*/
    }
};
// Function prototypes
loff_t pcd_lseek(struct file *filp, loff_t off, int whence);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *filp);
int check_permission(int access_mode, int dev_permission);
int pcd_platform_driver_probe(struct platform_device *pdev);
void pcd_platform_driver_remove(struct platform_device *pdev);

loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    int max_size = pcdev_data->size;

    loff_t temp = 0;

    pr_info("lseek called\n");
    pr_info("current file position = %lld\n", filp->f_pos);

    switch(whence)
    {
        case SEEK_SET:
            if((offset > max_size) || (offset < 0))
                return -EINVAL;
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = filp->f_pos + offset;
            if((temp > max_size) || (temp < 0))
                return -EINVAL;
            filp->f_pos = temp;
            break;
        case SEEK_END:
            temp = max_size + offset;
            if((temp > max_size) || (temp < 0))
                return -EINVAL;
            filp->f_pos = temp;
            break;
        default:
            return -EINVAL;
    }
    
    pr_info("new file position = %lld\n", filp->f_pos);

    return filp->f_pos;
}
ssize_t pcd_read (struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    int max_size = pcdev_data->size;

    //as per kernel documentation use zu for unsigned int
    pr_info("read called for %zu bytes \n", count);
    pr_info("current file position = %lld", *f_pos);

    /*Adjust the count*/
    if((*f_pos + count) > max_size)
        count = max_size - *f_pos;

    /*Copy to user*/
    if(copy_to_user(buff, pcdev_data->buffer + (*f_pos), count))
        return -EFAULT;

    /*update the current file operation*/
    *f_pos += count;

    /*Display operation details*/
    pr_info("number of bytes successfully read = %zu\n", count);
    pr_info("Updated file position = %lld", *f_pos);

    /*Return number of bytes successfully read*/
    return count;
}
ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    int max_size = pcdev_data->size;

    pr_info("write called for %zu bytes \n", count);
    pr_info("Current file position = %lld", *f_pos);

      /*Adjust the count*/
    if((*f_pos + count) > max_size)
      count = max_size - *f_pos;

    if(!count) 
    {
        return -ENOMEM;
        pr_err("No space left on device \n");
    }
    /*Copy to user*/
    if(copy_from_user(pcdev_data->buffer + (*f_pos), buff,  count))
        return -EFAULT;

    /*update the current file operation*/
    *f_pos += count;

    /*Display operation details*/
    pr_info("number of bytes successfully written = %zu\n", count);
    pr_info("Updated file position = %lld", *f_pos);
    return count;
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
    int ret;
    int minor_no;
    struct pcdev_private_data *pcdev_data;
    /* find out which device file atte,pted to open by user space */
    minor_no = MINOR(inode->i_rdev);
    pr_info("open called for device number %d\n", minor_no);
    pr_info("Device serial number = %s\n", pcdrv_data.pcdev_data[minor_no].serial_number);
    pr_info("Device permission = %d\n", pcdrv_data.pcdev_data[minor_no].permission);
    pr_info("Device buffer size = %d\n", pcdrv_data.pcdev_data[minor_no].size);
    pr_info("Device buffer address = %p\n", pcdrv_data.pcdev_data[minor_no].buffer);

    /*Get device's private data structure */
    pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);
    
    /*store private data to file pointer so that it can provided to other methods through filp*/
    filp->private_data = pcdev_data;

    /* check permission */
    ret =  check_permission(filp->f_flags, pcdev_data->permission);
    if(ret < 0)
    {
        pr_info("Permission denied\n");
        pr_err("Permission denied\n");
        return ret;
    }
    else if(ret == -EACCES)
    {
        pr_info("Permission denied\n");
        pr_err("Permission denied\n");
        return ret;
    }
    else
    {
        pr_info("Permission granted\n");
    }

    
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

    pr_info("matched platform device is found\n");
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
static int __init pcd_driver_nit(void)
{
    platform_driver_register(&pcd_platform_driver);
    pr_info("pcd platform driver loaded\n");
    return 0;
}

static void __exit pcd_driver_cleanup(void)
{
    platform_driver_unregister(&pcd_platform_driver);
    pr_info("pcd platform driver unloaded\n");
}

module_init(pcd_driver_nit);
module_exit(pcd_driver_cleanup);


MODULE_DESCRIPTION("Pseudo character driver");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_ALIAS("pcd");


