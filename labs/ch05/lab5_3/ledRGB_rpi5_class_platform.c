#include <linux/module.h>
#include <linux/platform_device.h>    // platform_driver_register(), platform_set_drvdata(), platform_get_resource()
#include <linux/fs.h>                // struct file_operations
#include <linux/io.h>                // devm_ioremap(), iowrite32()
#include <linux/leds.h>              // misc_register()
#include <linux/of.h>                // of_property_read_string()

//######### RPI5 addresses (BCM2712) ########
#define RP1_RW_OFFSET                   0x0000
#define RP1_XOR_OFFSET                  0x1000
#define RP1_SET_OFFSET                  0x2000
#define RP1_CLR_OFFSET                  0x3000

#define RP1_RIO_OUT                     0x00
#define RP1_RIO_OE                      0x04
#define RP1_RIO_IN                      0x08

#define GPIO_FUNCSEL_MASK               0b11111
#define PAD_OUT_DIS_MASK                (1 << 7)

#define GPIO_BASE_REG_ADDR              0x1f000d0000
#define RIO_BASE_REG_ADDR               0x1f000d0000
#define PAD_BASE_REG_ADDR               0x1f000d0000
#define REG_SIZE                         0x1000

struct led_dev {
    u32 led_mask;              // Different masks for each LED (R, G, B)
    void __iomem *base;
    struct led_classdev cdev;
};

static void led_control(struct led_classdev *led_cdev, enum led_brightness b)
{
    struct led_dev *dev = container_of(led_cdev, struct led_dev, cdev);
    iowrite32(0, dev->base);
    if (b != LED_OFF)   // LED ON
        iowrite32(dev->led_mask, dev->base + RP1_SET_OFFSET);
    else
        iowrite32(0, dev->base + RP1_CLR_OFFSET);    // LED OFF
}

static int ledclass_probe(struct platform_device *pdev)
{
    //void __iomem *base;
    void __iomem *g_ioremap_addr;
    struct device_node *child;
    struct device *dev = &pdev->dev;
    int ret = 0;
    int count;
    struct resource *r;
    pr_info("LED platform_probe enter\n");

    /* Get the first memory resource from the device tree */
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!r) {
        pr_err("IORESOURCE_MEM, 0 does not exist\n");
        return -EINVAL;
    }

    pr_info("r->start == 0x%08lx\n", (long unsigned int)r->start);
    pr_info("r->end == 0x%08lx\n", (long unsigned int)r->end);

    /* Map the memory region */
     g_ioremap_addr = devm_ioremap(dev, r->start, resource_size(r));
    if (!g_ioremap_addr) {
        pr_err("Failed to ioremap\n");
        return -ENOMEM;
    }

    count = of_get_child_count(dev->of_node);
    if (!count)
        return -ENOMEM;

    pr_info("There are %d nodes\n", count);

    // Adjust the GPIO function select register
    u32 GPFSEL_read = ioread32(g_ioremap_addr + RP1_RIO_IN + RP1_RIO_OUT);  // Read actual value (BCM2712)
    u32 GPFSEL_write = (GPFSEL_read & ~GPIO_FUNCSEL_MASK) | (GPIO_FUNCSEL_MASK);  // Example of writing GPIO functionality (Adjust for actual)

    // Set Direction of the GPIOs to Output
    iowrite32(GPFSEL_write, g_ioremap_addr + RP1_RIO_OE);

    // Switch OFF all LEDs
    iowrite32(GPIO_FUNCSEL_MASK, g_ioremap_addr + RP1_CLR_OFFSET);

    for_each_child_of_node( dev->of_node,child ) {
        struct led_dev *led_device;
        struct led_classdev *cdev;
        led_device = devm_kzalloc(&pdev->dev, sizeof(struct led_dev), GFP_KERNEL);
        if (!led_device)
            return -ENOMEM;

        cdev = &led_device->cdev;
        led_device->base = g_ioremap_addr;
        of_property_read_string(child, "label", &led_device->cdev.name);

        if (strcmp(led_device->cdev.name, "red") == 0) {
            led_device->led_mask = (1 << 27);  // GPIO_27 for red
        } else if (strcmp(led_device->cdev.name, "green") == 0) {
            led_device->led_mask = (1 << 22);  // GPIO_22 for green
        } else if (strcmp(led_device->cdev.name, "blue") == 0) {
            led_device->led_mask = (1 << 26);  // GPIO_26 for blue
        } else {
            pr_info("Bad device tree value\n");
            return -EINVAL;
        }

        /* Disable the timer trigger */
        led_device->cdev.brightness = LED_OFF;
        led_device->cdev.brightness_set = led_control;

        ret = devm_led_classdev_register(dev, &led_device->cdev);
        if (ret) {
            dev_err(dev, "Failed to register the LED\n");
            of_node_put(child);
            return ret;
        }
    }

    pr_info("LED platform_probe exit\n");
    return 0;
}

static int ledclass_remove(struct platform_device *pdev)
{
    pr_info("LED remove enter\n");
    pr_info("LED remove exit\n");
    return 0;
}

static const struct of_device_id my_of_ids[] = {
    { .compatible = "arrow,RGBclassleds" },
    {},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
    .probe = ledclass_probe,
    .remove = ledclass_remove,
    .driver = {
        .name = "RGBclassleds",
        .of_match_table = my_of_ids,
        .owner = THIS_MODULE,
    },
};

static int ledRGBclass_init(void)
{
    int ret_val;
    pr_info("LED demo_init enter\n");

    ret_val = platform_driver_register(&led_platform_driver);
    if (ret_val != 0) {
        pr_err("platform value returned %d\n", ret_val);
        return ret_val;
    }

    pr_info("LED demo_init exit\n");
    return ret_val;
}

static void ledRGBclass_exit(void)
{
    pr_info("LED driver enter\n");
    platform_driver_unregister(&led_platform_driver);
    pr_info("LED driver exit\n");
}

module_init(ledRGBclass_init);
module_exit(ledRGBclass_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is a driver that turns on/off RGB LEDs using the LED subsystem");
MODULE_AUTHOR("Mohamed Osman <mohamed.abdoun83@gmail.com>");
