obj-m := pcd_device_setup.o pcd_platform_driver.o

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
#KDIR=/lib/modules/$(shell uname -r)/build
KERN_DIR=/home/prashant/workspace/ldd/source/linux-5.10.168-ti-rt-r76/
HOST_KERN_DIR=/lib/modules/$(shell uname -r)/build/
all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules
#run on host machine#header is licated at /lib/modules/Kernerl version

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean

help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) help

host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules
	