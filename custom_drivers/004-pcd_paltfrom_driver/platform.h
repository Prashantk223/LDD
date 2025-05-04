struct pcdev_platform_data
{
    unsigned size;
    const char *serial_number;
    int permission;
};

#define RDWR 0x11
#define RD 0x01
#define WR 0x10