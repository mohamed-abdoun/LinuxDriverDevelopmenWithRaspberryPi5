#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/fs.h>
#include <linux/of.h>

/*
 * Store the global parameters here
 */
struct blinkm {
	u8			color[3];
	struct i2c_client	*client;
};

/* Set an blinkm_led struct for each of 3 leds */
struct blinkm_led {
	int			index;
	struct led_classdev	cdev;
	struct blinkm		*blinkm;
};

/*
 * This is the function that is called wen you write the brighness
 * under each device.
 */
static int blinkm_led_control(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct blinkm_led *led;
	struct blinkm *blinkm;

	led = container_of(led_cdev, struct blinkm_led, cdev);
	blinkm = led->blinkm;

	blinkm->color[led->index] = value;
	return i2c_smbus_write_i2c_block_data(blinkm->client, 0x6e, 3, blinkm->color);
}

static int blinkm_led_index(const char *name)
{
	if (strcmp(name, "red") == 0)
		return 0;
	else if (strcmp(name, "green") == 0)
		return 1;
	else if (strcmp(name, "blue") == 0)
		return 2;

	return -EINVAL;
}

static int blinkm_probe(struct i2c_client *client)
{
	struct fwnode_handle *child;
	struct device *dev = &client->dev;
	struct blinkm *blinkm;
	int ret;

	blinkm = devm_kzalloc(dev, sizeof(*blinkm), GFP_KERNEL);
	if (!blinkm)
		return -ENOMEM;

	blinkm->client = client;
	
	device_for_each_child_node(dev, child) {
		struct blinkm_led *led;
		struct led_classdev *cdev;

		led = devm_kzalloc(dev, sizeof(*led), GFP_KERNEL);
		if (!led)
			return -ENOMEM;

		led->blinkm = blinkm;
		cdev = &led->cdev;
		ret = fwnode_property_read_string(child, "label", &cdev->name);
		if (ret)
			return ret;
		led->index = blinkm_led_index(cdev->name);
		if (led->index < 0)
			return -EINVAL;

		cdev->max_brightness = 255;
		cdev->brightness_set_blocking = blinkm_led_control;
		devm_led_classdev_register(dev, cdev);
	}

	/* Stop running script */
	i2c_smbus_write_byte(client, 0x6f);
	/* Turn off leds */
	i2c_smbus_write_i2c_block_data(client, 0x6e, 3, blinkm->color);

	return 0;
}

static void blinkm_remove(struct i2c_client *client)
{
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "thingm,blinkm" },
	{ },
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct i2c_driver blinkm_driver = {
	.probe		= blinkm_probe,
	.remove		= blinkm_remove,
	.driver		= {
		.name		= "blinkm",
		.of_match_table	= my_of_ids,
		.owner		= THIS_MODULE,
	},
};

module_i2c_driver(blinkm_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Khalfella <khalfella@gmail.com>");
MODULE_DESCRIPTION("A driver to control blinkm i2c device");