obj-m := helloworld.o
KERNEL_DIR ?=/root/rpi/linux
all:
    make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) modules

clean:
   make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) clean
