obj-m := ledRGB_rpi5_class_platform.o ledRGB_rpi5_class_platform_using_lab5_2.o ledRGB_rpi5_class_platform_after_fixing_DT.o
KERNEL_DIR ?=/root/rpi5/linux

all:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) clean

deploy:
	scp ledRGB_rpi5_class_platform.ko ledRGB_rpi5_class_platform_using_lab5_2.ko ledRGB_rpi5_class_platform_after_fixing_DT.ko root@192.168.4.41:/root
