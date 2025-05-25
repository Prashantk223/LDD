#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"

#undef pr_fmt
//#define pr_fmt(fmt) fmt
#define pr_fmt(fmt) "%s:" fmt, __func__

void pcdev_release(struct device *dev)
{
    pr_info("Device released\n");
}
/*1. Create 2 platform   */
struct pcdev_platform_data pcdev_pdata[2] = {
    {
        .size = 512,
        .serial_number = "PCD1",
        .permission = RDWR, /*Read-Write Only*/
    },
    {
        .size = 1024,
        .serial_number = "PCD2",
        .permission = RDWR, /*Read-Write only*/
    }
};

struct pcdev_private_data
{
    struct pcdev_platform_data pdata;
    char *buffer;
    dev_t dev_num;
    struct cdev cdev;
}

/* 2. Create 2 platfrom devices */

struct platform_device platform_pcdev1 = {
    .name = "psuedo_char_device",
    .id = 0,
    .dev = {
        .release = pcdev_release, /* this is called when device is removed */
        .platform_data = &pcdev_pdata[0],
    },
};
struct platform_device platform_pcdev2 = 
{
    .name = "psuedo_char_device",
    .id = 1,
    .dev = {
        .release = pcdev_release,
        .platform_data = &pcdev_pdata[1],
    },
};

static int __init pcdev_platform_init(void)
{
    int ret;

    /* 3. Register the platform device */
    ret = platform_device_register(&platform_pcdev1);
    if (ret) {
        pr_err("Failed to register platform device 1\n");
        return ret;
    }

    ret = platform_device_register(&platform_pcdev2);
    if (ret) {
        pr_err("Failed to register platform device 2\n");
        platform_device_unregister(&platform_pcdev1);
        return ret;
    }

    pr_info("Platform devices setup module registered successfully\n");
    return 0;
}

static void __exit pcdev_platform_exit(void)
{
    /* 4. Unregister the platform device */
    platform_device_unregister(&platform_pcdev1);
    platform_device_unregister(&platform_pcdev2);

    pr_info("Platform devices unregistered successfully\n");
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Prashant");
MODULE_DESCRIPTION("Module which registers platform devices");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:psuedo_char_device");