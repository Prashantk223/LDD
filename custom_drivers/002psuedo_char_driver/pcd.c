#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h> // Include this header for cdev-related functionality
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#undef pr_fmt
//#define pr_fmt(fmt) fmt
#define pr_fmt(fmt) "%s:" fmt, __func__

#define DEV_MEM_SIZE 512
char device_buffer[DEV_MEM_SIZE];
dev_t device_number;

//cdev variables
struct cdev pcd_cdev;

// Function prototypes
loff_t pcd_lseek(struct file *filp, loff_t off, int whence);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *filp);

loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
    pr_info("lseek called\n");
    pr_info("current file position = %lld\n", filp->pos);

    loff_t temp = 0;
    switch(whence)
    {
        case SEEK_SET:
            if((offset > DEV_MEM_SIZE) || (offset < 0))
                return -EINVAL;
            filp->pos = offset;
        case SEEK_CUR:
            temp = filp->pos + offset;
            if((temp > DEV_MEM_SIZE) || (temp < 0))
                return -EINVAL;
            filp->pos = temp;
        case SEEK_END:
            temp = DEV_MEM_SIZE + offset;
            if((temp > DEV_MEM_SIZE) || (temp < 0))
                return -EINVAL;
            filp->pos = temp;
        default:
            return -EINVAL;
    }
    
    pr_info("new file position = %lld\n", filp->pos);

    return filp->pos;
}
ssize_t pcd_read (struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    //as per kernel documentation use zu for unsigned int
    pr_info("read called for %zu bytes \n", count);
    pr_info("current file position = %lld", *f_pos);

    /*Adjust the count*/
    if((*f_pos + count) > DEV_MEM_SIZE)
        count = DEV_MEM_SIZE - *f_pos;

    /*Copy to user*/
    if(copy_to_user(buff, &device_buffer[*f_pos], count))
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
    pr_info("write called for %zu bytes \n", count);
    pr_info("write called for %zu bytes \n", count);
    pr_info("Current file position = %lld", *f_pos);

      /*Adjust the count*/
    if((*f_pos + count) > DEV_MEM_SIZE)
      count = DEV_MEM_SIZE - *f_pos;

    if(!count) 
        return -ENOMEM;

    /*Copy to user*/
    if(copy_from_user(buff, &device_buffer[*f_pos], count))
        return -EFAULT;

    /*update the current file operation*/
    *f_pos += count;

    /*Display operation details*/
    pr_info("number of bytes successfully written = %zu\n", count);
    pr_info("Updated file position = %lld", *f_pos);
    return 0;
}
int pcd_open (struct inode *inode, struct file *filp)
{
    pr_info("open called\n");
    
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

struct class *pcd_class = NULL;
struct device *device_pcd = NULL;


static int __init pcd_driver_nit(void)
{
    int ret;
    //1. dynamicallly allocate a device number
    ret = alloc_chrdev_region(&device_number, 0, 1, "pcd_devices" );
    if(ret < 0)
        goto out;
    pr_info("Device number <major>:<number> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

    //2. Initialize the cdev structure with fops

    cdev_init(&pcd_cdev, &pcd_fops);

    //3. Register a device(cdev structure) with VFS
    pcd_cdev.owner = THIS_MODULE;
    ret = cdev_add(&pcd_cdev, device_number, 1);
    if(ret < 0)
        goto unreg_chrdev;

    //4. Create a device class
    // /sysis/pcd_class/pcd_device/device_number
    pcd_class = class_create("pcd_class");
    /*In case of error NULL is not returned instead ERROR pointer is returned. Error value
    in converted to void pointer type*/
    if(IS_ERROR(pcd_class))
    {
        pr_err("Class creation failed\n");
        ret = PTR_ERR(pcd_class); /*Extract error using macro*/
        goto cdev_failed;
    }     
    //5. populate the sysfs with device configuration - /sysis/pcd_class/pcd_device/device_number
    device_pcd = device_create(pcd_class, NULL, device_number, NULL, "pcd");
    
    pr_info("Module Init was successful\n");

    return 0;

unreg_chrdev:
    unregister_chrdev_region(device_number, 1);

out:
    return ret;
}

static void __exit pcd_driver_cleanup(void)
{
    //6. Destroy the device
    device_destroy(pcd_class, device_number);

    //7. Destroy the class
    class_destroy(pcd_class);

    //8. Unregister the device
    cdev_del(&pcd_cdev);

    //9. Free the device number
    unregister_chrdev_region(device_number, 1);

    pr_info("Module Exit was successful\n");
    pr_info("Device number <major>:<number> = %d:%d\n", MAJOR(device_number), MINOR(device_number));
}

module_init(pcd_driver_nit);
module_exit(pcd_driver_cleanup);


MODULE_DESCRIPTION("Pseudo character driver");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_ALIAS("pcd");


