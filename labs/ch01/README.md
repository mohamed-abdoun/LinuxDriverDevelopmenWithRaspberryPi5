# Building the Linux kernel

* clone the kernel
```
git clone --depth=1 --branch rpi-6.6.y  https://github.com/raspberrypi/linux
cd linux/
#root ➜ ~/rpi5/linux (rpi-6.6.y) $ git checkout -b ch01-build-kernel
#Switched to a new branch 'ch01-build-kernel'

```

* Compile the kernel, modules and Device Tree files. First, apply the default configuration:

    ```
    cd linux
    KERNEL=kernel_2712
    make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2712_defconfig
    ```

* Configure the following kernel settings that will be needed during the development of the labs:
    ```
    make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- menuconfig
    ```
* from the menu choose the following, please note some of those may change.Save the configuration, and exit from menuconfig.:
```
    Device drivers >
        [*] SPI support --->
            <*> User mode SPI device driver support

    Device drivers >
        <*> Industrial I/O support --->
            -*- Enable buffer support within IIO
            -*- Industrial I/O buffering based on kfifo
            <*> Enable IIO configuration via configfs
            -*- Enable triggered sampling support
            <*> Enable software IIO device support
            <*> Enable software triggers support
                Triggers - standalone --->
                    <*> High resolution timer trigger
                    <*> SYSFS trigger

    Device drivers >
        <*> Userspace I/O drivers --->
             <*> Userspace I/O platform driver with generic IRQ handling

    Device drivers >
        Input device support --->
            -*- Generic input layer (needed for keyboard, mouse, ...)
            <*> Polled input device skeleton

    Device drivers >
        <*> Industrial I/O support > Humidity sensors --->
            <*> STMicroelectronics HTS221 sensor Driver
```

* Compile kernel, Device Tree files and modules in a single step:
```
make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image modules dtbs
```

## Install the kernel
### 1) Find your boot media
* First, run lsblk. Then, connect your boot media. Run lsblk again; the new device represents your boot media. You should see output similar to the following:
*  mount these partitions as mnt/boot and mnt/root, adjusting the partition letter to match the location of your boot media:
```
mkdir mnt
mkdir mnt/boot
mkdir mnt/root
sudo mount /dev/sdb1 mnt/boot
sudo mount /dev/sdb2 mnt/root
```
### 2) Istall kernel:
  #### 2.1) Install the kernel modules onto the boot media:
* For 64-bit kernels:
```
sudo env PATH=$PATH make -j12 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=mnt/root modules_install
```
  #### 2.2) install the kernel and Device Tree blobs into the boot partition, backing up your original kernel.
* Run the following commands to create a backup image of the current kernel, install the fresh kernel image, overlays, README, and unmount the partitions:
```
sudo cp mnt/boot/$KERNEL.img mnt/boot/$KERNEL-backup.img
sudo cp arch/arm64/boot/Image mnt/boot/$KERNEL.img
sudo cp arch/arm64/boot/dts/broadcom/*.dtb mnt/boot/
sudo cp arch/arm64/boot/dts/overlays/*.dtb* mnt/boot/overlays/
sudo cp arch/arm64/boot/dts/overlays/README mnt/boot/overlays/
sudo umount mnt/boot
sudo umount mnt/root
```


