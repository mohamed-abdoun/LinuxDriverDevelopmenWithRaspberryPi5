obj-m := led_rpi3_UIO_platform.o
KERNEL_DIR ?=/root/rpi5/linux
# Cross-compiler prefix for ARM64
CROSS_COMPILE ?= aarch64-linux-gnu-

all:
	@echo "=== Building kernel module ==="
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) modules
	@echo "=== Building user-space UIO app ==="
	$(CROSS_COMPILE)gcc -static -o UIO_app UIO_app.c

clean:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) clean
	$(RM) UIO_app

deploy:
	scp led_rpi3_UIO_platform.ko UIO_app  root@192.168.4.41:/root
