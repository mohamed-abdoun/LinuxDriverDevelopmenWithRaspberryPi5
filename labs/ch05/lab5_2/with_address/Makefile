obj-m := ledRGB_rpi3_platform.o
KERNEL_DIR ?=/root/rpi5/linux

all:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) clean

deploy:
	scp ledRGB_rpi3_platform.ko  root@192.168.4.41:/root
