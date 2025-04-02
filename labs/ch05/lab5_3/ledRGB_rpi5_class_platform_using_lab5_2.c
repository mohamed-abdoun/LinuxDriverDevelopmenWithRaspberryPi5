#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>                // struct file_operations
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/leds.h>              // misc_register()

#define RP1_RW_OFFSET                   0x0000
#define RP1_XOR_OFFSET                  0x1000
#define RP1_SET_OFFSET                  0x2000
#define RP1_CLR_OFFSET                  0x3000

#define RP1_RIO_OUT                     0x00
#define RP1_RIO_OE                      0x04
#define RP1_RIO_IN                      0x08

#define	GPIO_FUNCSEL_MASK		0b11111
#define PAD_OUT_DIS_MASK		1<<7

#define GPIO_BASE_REG_ADDR		0x1f000d0000
#define RIO_BASE_REG_ADDR		0x1f000e0000
#define PAD_BASE_REG_ADDR		0x1f000f0000
#define REG_SIZE			0xc000

struct led_device {
    u32 led_mask; /* there are different masks if led is R,G or B */
	void __iomem		*gpio;
	void __iomem		*rio;
	void __iomem		*pad;
	struct miscdevice	misc;
	const char		*name;
	int			pin;
	struct led_classdev cdev;
};

static void __iomem *GPIO_BASE_V;
static void __iomem *RIO_BASE_V;
static void __iomem *PAD_BASE_V;

static ssize_t led_write(struct file *file, const char __user *buff,
			 size_t count, loff_t *ppos)
{
	struct led_device *dev;
	int val, ret, offset;

	dev = container_of(file->private_data, struct led_device, misc);
	ret = kstrtoint_from_user(buff, count, 10, &val);
	if (ret)
		return ret;
	offset = val ? RP1_SET_OFFSET : RP1_CLR_OFFSET;
	iowrite32(1 << dev->pin, dev->rio + offset + RP1_RIO_OUT);
	return count;
}

static ssize_t led_read(struct file *file, char __user *buff, size_t count,
		        loff_t *ppos){
	return 0;
}

static const struct file_operations ledrgb_fops = {
	.owner = THIS_MODULE,
	.write = led_write,
	.read = led_read,
};

static void ledrgb_init_dev(struct led_device *dev)
{
	/*
	 * Each GPIO has two registers of 32bit each, so we need
	 * to multiply GPIO number with that. We add 4 to point
	 * to GPIO_XX_CTRL register.
	 */
	dev->gpio = GPIO_BASE_V + dev->pin * sizeof(u32) * 2 + 4;

	/*
	 * Take RIO_BASE_V for now. Will calculate offsets as needed
	 */
	dev->rio = RIO_BASE_V;

	/*
	 * Add 4 to skip VOLTAGE_SELECT register. Every GPIO has
	 * one 32bit register so we account for that.
	 */
	dev->pad = PAD_BASE_V + 4 + dev->pin * sizeof(u32);

	/*
	 * We are using sys_rio as the peripheral that controls this GPIO.
	 * Before we set the muxing, let us clear OUT of sys_rio and set
	 * OE (Output Enable) on sys_rio.
	 *
	 * This gives us two things:
	 * - Default LED off since OUT is cleared.
	 * - Signal GPIO to set pin direction to output.
	 *
	 * Setup sys_rio such that dev->pin is set to
	 * output and initialize its value to 1
	 */
	iowrite32(1 << dev->pin, dev->rio + RP1_CLR_OFFSET + RP1_RIO_OUT);
	iowrite32(1 << dev->pin, dev->rio + RP1_SET_OFFSET + RP1_RIO_OE);

	/*
	 * Setting the GPIO registers focuses on three thigs:
	 * - Setting FUNCSEL to 0x5 so that GPIO uses sys_rio
	 * - Setting OUTOVER (Output Override) to 0 so that GPIO
	 *   output is taken straight from sys_rio OUT
	 * - Setting OEOVER (Output Enable Override) to 0 so that
	 *   GPIO uses OE from the peripheral (sys_rio) in this case.
	 *
	 * We need to connect sys_rio and GPIO this way:
	 * - OE  (sys_rio) --> OEOVER  (GPIO)
	 * - OUT (sys_rio) --> OUTOVER (GPIO)
	 *
	 * Both OEOVER and OUTOVER default to zero. Let us just
	 * use the default value. Ideally we should set them but
	 * I do not feel like doing it now. Feel free to do that!
	 */
	u32 ctrl = ioread32(dev->gpio);
	ctrl = (ctrl & (~GPIO_FUNCSEL_MASK)) | 0b00101;
	iowrite32(ctrl, dev->gpio);

	/*
	 * Clear OD (Output disable) bit in pad register. This bit
	 * defaults to 1. Clear it so we can set output voltage.
	 */
	u32 pad = ioread32(dev->pad);
	pad = pad & (~PAD_OUT_DIS_MASK);
	iowrite32(pad, dev->pad);
}


static int ledrgb_get_device_pin(const char *name)
{
    pr_info("LED ledrgb_get_device_pin  %s\n", name );
	if (strcmp(name, "red") == 0)
		return 22;
	if (strcmp(name, "green") == 0)
		return 26;
	if (strcmp(name, "blue") == 0)
		return 27;

	 pr_info("LED ledrgb_get_device_pin  Could not find  %s\n",name );
	return -EINVAL;
}


static void led_control(struct led_classdev *led_cdev, enum led_brightness b)
{
     pr_info("LED_control received brightness = %u\n", b);
     // struct led_dev *led = container_of(led_cdev, struct led_dev, cdev);
}
static int ledrgb_probe(struct platform_device *pdev)
{
	struct led_device *led_device;
	struct led_classdev *classdev;
	struct device_node *child;

    for_each_child_of_node(pdev->dev.of_node, child) {
        const char *label;

        led_device = devm_kzalloc(&pdev->dev, sizeof(*led_device), GFP_KERNEL);
        if (!led_device)
            return -ENOMEM;

        classdev = &led_device->cdev;

        if (of_property_read_string(child, "label", &label)) {
                pr_err("Child node missing 'label'\n");
                continue;
            }

        led_device->name = label;
        led_device->pin = ledrgb_get_device_pin(led_device->name);
        if (led_device->pin < 0) {
           pr_err("Invalid label: %s\n", led_device->name);
           continue;
        }

        ledrgb_init_dev(led_device);

        led_device->misc.minor = MISC_DYNAMIC_MINOR;
        led_device->misc.name = led_device->name;
        led_device->misc.fops = &ledrgb_fops;

        if (misc_register(&led_device->misc)) {
            pr_err("Failed to register device %s\n", led_device->name);
            continue;
        }




         /* Disable the timer trigger */
         led_device->cdev.brightness = LED_OFF;
         led_device->cdev.brightness_set = led_control;

         pr_info("=== RGB LED Probe ===\n");
         pr_info("  name  : %s\n", led_device->name);
         pr_info("  pin   : %d\n", led_device->pin);
         pr_info("  gpio  : %px\n", led_device->gpio);
         pr_info("  rio   : %px\n", led_device->rio);
         pr_info("  pad   : %px\n", led_device->pad);
         pr_info("  brightness   : %d\n",   led_device->cdev.brightness);
         pr_info("  brightness_set  : %px\n",led_device->cdev.brightness_set);




    }



	return 0;
}

static int ledrgb_remove(struct platform_device *pdev)
{
	struct led_device *dev;
	struct device_node *child;
     for_each_child_of_node(pdev->dev.of_node, child) {
        const char *label;
        pr_info("LED - ledrgb_remove-  Deregistered RGB LED: %s (pin %d)\n", dev->name, dev->pin);
         dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
         if (!dev)
            continue;

         if (of_property_read_string(child, "label", &label)) {
              pr_err("Child node missing 'label'\n");
               continue;
         }

         dev->name = label;
         dev->pin = ledrgb_get_device_pin(dev->name);
         if (dev->pin < 0) {
             pr_err("Invalid label: %s\n", dev->name);
             continue;
         }

        misc_deregister(&dev->misc);
     }

	 pr_info("LED - ledrgb_remove- Deregistered all devices");
	return 0;
}

static const struct of_device_id of_ids[] = {
	{ .compatible = "arrow,RGBclassleds" },
	{},
};
MODULE_DEVICE_TABLE(of, of_ids);

static struct platform_driver led_platform_driver = {
	.probe = ledrgb_probe,
	.remove = ledrgb_remove,
	.driver = {
		.name =  "RGBclassleds",
		.of_match_table = of_ids,
		.owner = THIS_MODULE,
	},
};

static int __init ledrgb_init(void)
{
	int ret;

	GPIO_BASE_V = ioremap(GPIO_BASE_REG_ADDR, REG_SIZE);
	if (!GPIO_BASE_V)
		return -EINVAL;

	RIO_BASE_V = ioremap(RIO_BASE_REG_ADDR, REG_SIZE);
	if (!RIO_BASE_V)
		goto unmap_gpio_base;

	PAD_BASE_V = ioremap(PAD_BASE_REG_ADDR, REG_SIZE);

	if (!PAD_BASE_V)
		goto unmap_rio_base;

	ret = platform_driver_register(&led_platform_driver);
	if (ret)
		goto unmap_pad_base;



	return 0;

unmap_pad_base:
	iounmap(PAD_BASE_V);
unmap_rio_base:
	iounmap(RIO_BASE_V);
unmap_gpio_base:
	iounmap(GPIO_BASE_V);
	return -EINVAL;
}

static void __exit ledrgb_exit(void)
{
	iounmap(GPIO_BASE_V);
	iounmap(RIO_BASE_V);
	iounmap(PAD_BASE_V);
	platform_driver_unregister(&led_platform_driver);
}

module_init(ledrgb_init);
module_exit(ledrgb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Khalfella <khalfella@gmail.com>");
MODULE_DESCRIPTION("Turn an LED on GPIO on/off by accessing RP1 registers");