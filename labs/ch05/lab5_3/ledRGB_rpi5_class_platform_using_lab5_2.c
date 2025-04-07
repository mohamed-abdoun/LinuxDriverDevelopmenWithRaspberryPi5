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

#define REG_SIZE			0xc000

struct led_device {
    u32 led_mask; /* there are different masks if led is R,G or B */
	void __iomem		*gpio;
	void __iomem		*rio;
	void __iomem		*pad;
	const char		*name;
	int			pin;
	struct led_classdev cdev;
};

static void __iomem *GPIO_BASE_V;
static void __iomem *RIO_BASE_V;
static void __iomem *PAD_BASE_V;

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
   // 27 (red)--22 (green)-- 26 blue
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
	struct led_device *dev;

     pr_info("LED_control received brightness = %u\n", b);
     dev = container_of(led_cdev, struct led_device, cdev);
	 //writing brightness
	int offset = b==LED_ON ? RP1_SET_OFFSET : RP1_CLR_OFFSET;
	iowrite32(1 << dev->pin, dev->rio + offset + RP1_RIO_OUT);

}
static int ledrgb_probe(struct platform_device *pdev)
{
	struct led_device *led_device;
	struct led_classdev *classdev;
	struct device_node *child;

	struct resource *r;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	GPIO_BASE_V = devm_ioremap(&pdev->dev, r->start, resource_size(r));

	r = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	RIO_BASE_V = devm_ioremap(&pdev->dev, r->start, resource_size(r));

	r = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	PAD_BASE_V = devm_ioremap(&pdev->dev, r->start, resource_size(r));



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

         /* Disable the timer trigger */
         led_device->cdev.name = label;
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

    	int ret = devm_led_classdev_register(&pdev->dev, &led_device->cdev);
    	if (ret) {
    		dev_err(&pdev->dev, "Failed to register the LED\n");
    		of_node_put(child);
    		return ret;
    	}

    }

	return 0;
}

static int ledrgb_remove(struct platform_device *pdev)
{
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

module_platform_driver(led_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Khalfella <khalfella@gmail.com>");
MODULE_DESCRIPTION("Turn an LED on GPIO on/off by accessing RP1 registers");