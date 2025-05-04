#host PC
#make -C /lib/modules/$(uname -r)/build/ M=$(pwd) modules

#target BBB
sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C /home/prashant/workspace/ldd/source/linux-5.10.168-ti-rt-r76/ M=$PWD modules