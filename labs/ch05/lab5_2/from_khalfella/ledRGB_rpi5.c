#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>

struct led_dev {
	struct gpio_desc	*gpio;
	struct miscdevice	misc;
	int			value;
	const char		*label;
};

static ssize_t led_write(struct file *file, const char __user *buff,
			 size_t count, loff_t *ppos)
{
	struct led_dev *dev;
	int val, ret;

	dev = container_of(file->private_data, struct led_dev, misc);
	ret = kstrtoint_from_user(buff, count, 10, &val);
	if (ret)
		return ret;
	dev->value = !!val;
	gpiod_set_value(dev->gpio, dev->value);
	return count;
}

static ssize_t led_read(struct file *file, char __user *buff, size_t count,
		        loff_t *ppos)
{
	return 0;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.write = led_write,
	.read = led_read,
};

static int led_probe(struct platform_device *pdev)
{
	struct led_dev *dev;
	int ret;

	dev = devm_kzalloc(&pdev->dev, sizeof(struct led_dev), GFP_KERNEL);
	if (!dev) {
		pr_err("failed to allocate device\n");
		return -ENOMEM;
	}

	dev->gpio = devm_gpiod_get(&pdev->dev, NULL, 0);
	if (!dev->gpio) {
		pr_err("failed to get gpio\n");
		return -ENOMEM;
	}

	ret = of_property_read_string(pdev->dev.of_node, "label", &dev->label);
	if (ret) {
		pr_err("failed read led label string, ret = %d\n", ret);
		return ret;
	}

	ret = gpiod_direction_output(dev->gpio, dev->value);
	if (ret) {
		pr_err("failed to set gpio direction, ret = %d\n", ret);
		return ret;
	}

	dev->misc.minor = MISC_DYNAMIC_MINOR;
	dev->misc.name = dev->label;
	dev->misc.fops = &fops;
	ret = misc_register(&dev->misc);
	if (ret) {
		pr_err("failed to register misc device, ret = %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, dev);
	return 0;
}

static int led_remove(struct platform_device *pdev)
{
	struct led_dev *dev = platform_get_drvdata(pdev);

	misc_deregister(&dev->misc);
	gpiod_set_value(dev->gpio, 0);
	return 0;
}

static const struct of_device_id of_ids[] = {
	{ .compatible = "arrow,RGBleds" },
	{},
};
MODULE_DEVICE_TABLE(of, of_ids);

static struct platform_driver led_platform_driver = {
	.probe = led_probe,
	.remove = led_remove,
	.driver = {
		.name = "RGBleds",
		.of_match_table = of_ids,
		.owner = THIS_MODULE,
	},
};

module_platform_driver(led_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Khalfella <khalfella@gmail.com>");
MODULE_DESCRIPTION("Turn an LED on GPIO on/off");