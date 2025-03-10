# Building the Linux kernel

* clone the kernel
```
git clone --depth=1 --branch rpi-6.6.y  https://github.com/raspberrypi/linux
cd linux/
# git checkout -b your_branch
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
        [*] SPI support --->
            <*> User mode SPI device driver support
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
make -j6 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu Image modules dtbs
```

