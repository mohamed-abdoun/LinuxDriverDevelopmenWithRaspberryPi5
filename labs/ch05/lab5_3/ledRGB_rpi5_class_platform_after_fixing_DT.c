#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/io.h>

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
	void __iomem		*gpio;
	void __iomem		*rio;
	void __iomem		*pad;
	struct miscdevice	misc;
	const char		*name;
	int			pin;
};

static void __iomem *GPIO_BASE_V;
static void __iomem *RIO_BASE_V;
static void __iomem *PAD_BASE_V;

static ssize_t led_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos)
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

static ssize_t led_read(struct file *file, char __user *buff, size_t count,loff_t *ppos){
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

	if (strcmp(name, "ledred") == 0)
		return 27;
	if (strcmp(name, "ledgreen") == 0)
		return 22;
	if (strcmp(name, "ledblue") == 0)
		return 26;

	pr_info("LED ledrgb_get_device_pin  Could not find  %s\n",name );
	return -EINVAL;
}

static int ledrgb_probe(struct platform_device *pdev)
{
	struct led_device *dev;
	int ret;

	dev = devm_kzalloc(&pdev->dev, sizeof(struct led_device), GFP_KERNEL);
	if (!dev) {
		pr_err("Failed to allocate led device\n");
		return -ENOMEM;
	}

	ret = of_property_read_string(pdev->dev.of_node, "label", &dev->name);
	if (ret) {
		pr_err("Failed to get led device name, err = %d\n", ret);
		return ret;
	}

	dev->pin = ledrgb_get_device_pin(dev->name);
	if (dev->pin < 0) {
		pr_err("Failed to get device pin, err = %d\n", dev->pin);
		return dev->pin;
	}

	ledrgb_init_dev(dev);

	dev->misc.minor = MISC_DYNAMIC_MINOR;
	dev->misc.name = dev->name;
	dev->misc.fops = &ledrgb_fops;

	ret = misc_register(&dev->misc);
	if (ret) {
		pr_err("Failed to initialize misc device err = %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, dev);
	return 0;
}

static int ledrgb_remove(struct platform_device *pdev)
{
	struct led_device *dev;

	dev = platform_get_drvdata(pdev);
	misc_deregister(&dev->misc);
	return 0;
}

static const struct of_device_id of_ids[] = {
	{ .compatible = "arrow,RGBleds" },
	{},
};
MODULE_DEVICE_TABLE(of, of_ids);

static struct platform_driver led_platform_driver = {
	.probe = ledrgb_probe,
	.remove = ledrgb_remove,
	.driver = {
		.name = "RGBclassleds",
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