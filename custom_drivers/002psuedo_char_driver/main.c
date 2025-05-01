#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h> // Include this header for cdev-related functionality
#include <linux/device.h>
#include <linux/kdev_t.h>

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

loff_t pcd_lseek (struct file *filp, loff_t off, int whence)
{
    pr_info("lseek called\n");
    return 0;
}
ssize_t pcd_read (struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("read called for %zu bytes \n", count);
    return 0;
}
ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("write called for %zu bytes \n", count);
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
    //1. dynamicallly allocate a device number
    (void)alloc_chrdev_region(&device_number, 0, 1, "pcd_devices" );

    pr_info("Device number <major>:<number> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

    //2. Initialize the cdev structure with fops

    cdev_init(&pcd_cdev, &pcd_fops);

    //3. Register a device(cdev structure) with VFS
    pcd_cdev.owner = THIS_MODULE;
    cdev_add(&pcd_cdev, device_number, 1);

    //4. Create a device class
    // /sysis/pcd_class/pcd_device/device_number
    pcd_class = class_create("pcd_class");

    //5. populate the sysfs with device configuration - /sysis/pcd_class/pcd_device/device_number
    device_pcd = device_create(pcd_class, NULL, device_number, NULL, "pcd_device");
    
    pr_info("Module Init was successfuln");

    return 0;
}

static void __exit pcd_driver_cleanup(void)
{
}

module_init(pcd_driver_nit);
module_exit(pcd_driver_cleanup);


MODULE_DESCRIPTION("Pseudo character driver");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_ALIAS("pcd");


