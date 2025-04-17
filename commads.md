# build the kernel
------------------
 

1) Download kernel source 

git clone --depth=1 --branch rpi-6.6.y  https://github.com/raspberrypi/linux 

 

2) Cross-compile the kernel 
 
```
sudo apt install bc bison flex libssl-dev make libc6-dev libncurses5-dev 
sudo apt install crossbuild-essential-arm64 
sudo apt install crossbuild-essential-armhf 
```

2.1) Build configuration 

sudo apt install libncurses5-dev 
Menuconfig 

To cross-compile a 64-bit kernel: 

`make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- menuconfig`

 

 

Configure the following kernel settings that will be needed during the development of the labs: 

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

  

For the LAB 12.1, you will need the functions that enable the triggered buffer support. If they are 

not defined accidentally by another driver, there's an error thrown out while linking. To solve 

this problem, you can select, for example, the HTS221 driver, which includes this triggered buffer 

support: 

Device drivers > 

<*> Industrial I/O support > Humidity sensors ---> 

<*> STMicroelectronics HTS221 sensor Driver 

Save the configuration, and exit from menuconfig. 

 


Save your changes 

Once you’re done making changes, press Escape until you’re prompted to save your new configuration. By default, this saves to the .config file. You can save and load configurations by copying this file. 

 
```
cd linux 
KERNEL=kernel_2712 
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2712_defconfig 
```
 

2.2) Build 
------------
 

To prevent the kernel from overwriting existing modules in /lib/modules and to clarify that you run your own kernel in uname output, adjust LOCALVERSION. 

To adjust LOCALVERSION, change the following line in .config: 

`CONFIG_LOCALVERSION="-v7l-MY_CUSTOM_KERNEL" `

 

Run the following command to build a 64-bit kernel: 

 

`make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image modules dtbs `

 

3) Install the kernel 

 

 

3.1) Find your boot media 

First, run lsblk. Then, connect your boot media. Run lsblk again; the new device represents your boot media. You should see output similar to the following: 

sdb 
   sdb1 
   sdb2 

If sdb represents your boot media, sdb1 represents the the FAT32-formatted boot partition and sdb2 represents the (likely ext4-formatted) root partition. 

First, mount these partitions as mnt/boot and mnt/root, adjusting the partition letter to match the location of your boot media: 
```
mkdir mnt 
mkdir 
mnt/boot 
mkdir mnt/root 
sudo mount /dev/sdb1 mnt/boot 
sudo mount /dev/sdb2 mnt/root 
```

Install 
--------

Next, install the kernel modules onto the boot media: 

For 64-bit kernels: 

`sudo env PATH=$PATH make -j12 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=mnt/root modules_install`

 










#compile device tree
--------------------

```
make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- dtbs
scp arch/arm64/boot/dts/broadcom/{bcm2712-rpi-5-b.dtb,bcm2712d0-rpi-5-b.dtb,bcm2712-rpi-500.dtb,bcm2712-rpi-cm5-cm5io.dtb,bcm2712-rpi-cm5-cm4io.dtb,bcm2712-rpi-cm5l-cm5io.dtb,bcm2712-rpi-cm5l-cm4io.dtb}  root@192.168.4.41:/boot/firmware/

```
