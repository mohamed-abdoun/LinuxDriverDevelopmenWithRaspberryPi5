obj-m := led_rpi3_UIO_platform.o
KERNEL_DIR ?=/root/rpi5/linux

all:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) clean

deploy:
	scp led_rpi3_UIO_platform.ko  root@192.168.4.41:/root
