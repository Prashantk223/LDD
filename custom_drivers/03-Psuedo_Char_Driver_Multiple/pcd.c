#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h> // Include this header for cdev-related functionality
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include <linux/version.h>

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
    char *buffer;
    unsigned size;
    const char *serial_number;
    int permission;
    struct cdev cdev;

};
/*Driver private data structure*/
struct pcdrv_private_data
{
    int total_devices;
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
    return 0;
}



int check_permission(int access_mode, int dev_permission)
{
    pr_info("Device permission = %d\n", dev_permission);
    pr_info("User permission = %d\n", user_permission);

    if((dev_permission & user_permission) == 0)
    {
        pr_err("Permission denied\n");
        return -EACCES;
    }
    else
    {
        pr_info("Permission granted\n");
        return 0;
    }
}
{
    return 0;
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
        pr_err("Permission denied\n");
        return ret;
    }
    else if(ret == -EACCES)
    {
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



static int __init pcd_driver_nit(void)
{
    int ret;
    int i;

    //1. dynamicallly allocate a device number
    ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NO_OF_DEVICES, "pcd_devices" );
    if(ret < 0)
    {
        pr_err("Device number allocation failed\n");
        pr_err("Error code = %d\n", ret);
        goto out;
    }
    //4. Create a class
    // /sysis/pcd_class/pcd_device/device_number
    #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
    pcdrv_data.pcd_class = class_create("pcd_class");
    #else
    pcd_class = class_create(HIS_MODULE, "pcd");
    #endif
    /*In case of error NULL is not returned instead ERROR pointer is returned. Error value
    in converted to void pointer type*/
    if(IS_ERR(pcdrv_data.pcd_class))
    {
        pr_err("Class creation failed\n");
        ret = PTR_ERR(pcdrv_data.pcd_class); /*Extract error using macro*/
        goto unreg_chrdev;
    }  

    for(i = 0; i < NO_OF_DEVICES; i++)
    {
        pr_info("Device number <major>:<number> = %d:%d\n", MAJOR(pcdrv_data.device_number + i), MINOR(pcdrv_data.device_number+i));
        /* 2. Initialize the cdev structure with fops */
        cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);

        /* 3. Register a device(cdev structure) with VFS */
        pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
        ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number + i, 1);
        if(ret != 0)
        {
            pr_err("cdev_add failed\n");
            goto cdev_del;
        }
    
        /* 5. populate the sysfs with device configuration - /sysis/pcd_class/pcd_device/device_number  */
        pcdrv_data.device_pcd = device_create(pcdrv_data.pcd_class, NULL, pcdrv_data.device_number + i, NULL, "pcdev-%d", i);
        if(IS_ERR(pcdrv_data.device_pcd))
        {
            pr_err("device creation failed\n");
            ret = PTR_ERR(pcdrv_data.device_pcd); /*Extract error using macro*/
            goto class_del;
        }  
    }

    pr_info("Module Init was successful\n");

    return 0;

cdev_del:
class_del:
    for(; i >= 0; i--)
    {
        device_destroy(pcdrv_data.pcd_class, pcdrv_data.device_number + i);
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }
    class_destroy(pcdrv_data.pcd_class);

unreg_chrdev:
    unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);

out:
    pr_info("Module Init failed\n");
    pr_info("Device number <major>:<number> = %d:%d\n", MAJOR(pcdrv_data.device_number), MINOR(pcdrv_data.device_number));
    pr_info("Error code = %d\n", ret);
    return ret;
}

static void __exit pcd_driver_cleanup(void)
{
    int i;
    //1. unregister the device
    for(i = 0; i < NO_OF_DEVICES; i++)
    {
        device_destroy(pcdrv_data.pcd_class, pcdrv_data.device_number + i);
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }  
    //2. destroy the class
    class_destroy(pcdrv_data.pcd_class);
    //3. unregister the device number
    unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);
    //4. free the memory if any
    //kfree(pcdrv_data.pcd_class);

    pr_info("Module Exit was successful\n");
}

module_init(pcd_driver_nit);
module_exit(pcd_driver_cleanup);


MODULE_DESCRIPTION("Pseudo character driver");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_ALIAS("pcd");


