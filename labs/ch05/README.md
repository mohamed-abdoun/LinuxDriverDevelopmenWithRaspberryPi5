* Modify the Device Tree files (located in arch/arm/boot/dts/ in the kernel source tree),in RPI5 this file localted under
```
vim arch/arm64/boot/dts/broadcom/bcm2712-rpi-5-b.dts
```
* Add below config to above file
```
    hellokeys {
         compatible = "arrow,hellokeys";
    };
```
* Build and deploy the module to the Raspberry Pi:
```angular2html
make 
make deploy
```

* Build the modified Device Tree, and load it to the target processor:
```
 # make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- dtbs
  DTC     arch/arm64/boot/dts/broadcom/bcm2712-rpi-5-b.dtb
  DTC     arch/arm64/boot/dts/broadcom/bcm2712d0-rpi-5-b.dtb
  DTC     arch/arm64/boot/dts/broadcom/bcm2712-rpi-500.dtb

# scp arch/arm64/boot/dts/broadcom/bcm2712-rpi-5-b.dtb  root@192.168.4.41:/boot/
# reboot
```

### demonstration
* See the Device Tree nodes under the root node:
```
root@rpi5:~# ls /proc/device-tree
'#address-cells'   clk-27M	      memory@0	        sd_io_1v8_reg
 aliases	   clocks	      memreserve        sd_vcc_reg
 arm-pmu	   compatible	      model	        serial-number
 axi		   cooling_fan	      name	       '#size-cells'
 cam0_clk	   cpus		      __overrides__     soc
 cam0_reg	   dummy	      phy	        __symbols__
 cam1_clk	   hvs@107c580000     psci	        system
 cam1_reg	   i2c0if	      pwr_button        thermal-zones
 cam_dummy_reg	   i2c0mux	      reserved-memory   timer
 chosen		   interrupt-parent   rp1_firmware      wl_on_reg
 clk-108M	   leds		      rp1_vdd_3v3

```
* Check the Raspberry Pi model ``cat /proc/device-tree/model``
* Load the module. The probe() function is called:
```
insmod hellokeys_rpi3

```

* See "hellokeys" sysfs entries:
```
# find /sys -name "*hellokeys*"
/sys/devices/platform/hellokeys
/sys/bus/platform/devices/hellokeys
/sys/bus/platform/drivers/hellokeys
/sys/bus/platform/drivers/hellokeys/hellokeys
/sys/firmware/devicetree/base/hellokeys
/sys/module/hellokeys_rpi5
/sys/module/hellokeys_rpi5/drivers/platform:hellokeys

# ======
ls -l /sys/bus/platform/drivers/hellokeys/
total 0
--w------- 1 root root 16384 Mar 10 01:24 bind
lrwxrwxrwx 1 root root     0 Mar 10 01:24 hellokeys -> ../../../../devices/platform/hellokeys
lrwxrwxrwx 1 root root     0 Mar 10 01:24 module -> ../../../../module/hellokeys_rpi5
--w------- 1 root root 16384 Mar 10 01:22 uevent
--w------- 1 root root 16384 Mar 10 01:24 unbind
# ======
 ls -l /sys/module/hellokeys_rpi5/drivers/
total 0
lrwxrwxrwx 1 root root 0 Mar 10 01:24 platform:hellokeys -> ../../../bus/platform/drivers/hellokeys
# ======
```

* Check that `mydev` is created under the misc class folder:
```
    ls /sys/class/misc/
autofs		 device-mapper	kvm	      rfkill	   watchdog
cachefiles	 fuse		loop-control  vcio
cpu_dma_latency  hw_random	mydev	      vga_arbiter

```

* See the assigned mayor and minor numbers for the device mydev. The mayor number 10 is assigned by
  the misc framework:
```
cat /sys/class/misc/mydev/dev
10:123
```

* Run the application ioctl_test:
```
./ioctl_test

```
