obj-m := lab11.2-nunchuk_provider.o lab11.2-nunchuk_comsumer.o
KERNEL_DIR ?=/root/rpi5/linux
# Cross-compiler prefix for ARM64
CROSS_COMPILE ?= aarch64-linux-gnu-

all:
	@echo "=== Building kernel module ==="
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- M=$(PWD) clean

deploy:
	scp lab11.2-nunchuk_provider.ko lab11.2-nunchuk_comsumer.ko root@192.168.4.41:/root
