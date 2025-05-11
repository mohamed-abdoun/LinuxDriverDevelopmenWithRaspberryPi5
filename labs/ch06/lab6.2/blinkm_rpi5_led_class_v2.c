#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/slab.h>

/* Set an led_device struct for each of the 5 led devices */
struct led_device
{
    u8 brightness;
    struct led_classdev cdev;
    struct led_priv *private;
};

/*
 * Store the global parameters shared for the 5 led devices.
 * The parameters are updated after each led_control() call
 */
struct led_priv
{
    u32 num_leds;
    u8 cmd_buf[6]; /* scratch buffer for BlinkM commands */
    struct i2c_client *client;
};

static int blinkm_send(struct i2c_client *client, u8 *buf, int len)
{
    int ret = i2c_master_send(client, buf, len);
    return (ret == len) ? 0 : (ret < 0 ? ret : -EIO);
}

static int blinkm_cmd_now(struct i2c_client *client, u8 R, u8 G, u8 B)
{
    u8 cmd[4] = {'n', R, G, B};
    return blinkm_send(client, cmd, sizeof(cmd));
}

static int blinkm_cmd_fade(struct i2c_client *client, u8 R, u8 G, u8 B)
{
    u8 cmd[4] = {'c', R, G, B};
    return blinkm_send(client, cmd, sizeof(cmd));
}

static int __maybe_unused blinkm_cmd_stop(struct i2c_client *client)
{
    u8 cmd = 'o';
    return blinkm_send(client, &cmd, 1);
}

static int led_control(struct led_classdev *cdev, enum led_brightness value)
{
    struct led_device *leddev = container_of(cdev, struct led_device, cdev);
    struct led_priv *priv = leddev->private;

    if (value > 255)
        return -EINVAL;

    /* Simple: map brightness of each virtual LED to BlinkM RGB channels */
    switch (cdev->name[0])
    {
    case 'r': /* red */
        return blinkm_cmd_now(priv->client, value, 0, 0);
    case 'g': /* green */
        return blinkm_cmd_now(priv->client, 0, value, 0);
    case 'b': /* blue */
        return blinkm_cmd_now(priv->client, 0, 0, value);
    case 'm': /* main: map to white scale */
        return blinkm_cmd_now(priv->client, value, value, value);
    case 's': /* sub: do a fade */
        return blinkm_cmd_fade(priv->client, value, 255 - value, value);
    default:
        return -EINVAL;
    }
}

static int blinkm_probe(struct i2c_client *client)
{
    struct led_priv *priv;
    struct fwnode_handle *child;
    int ret;

    priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    priv->client = client;
    priv->num_leds = 0;
    i2c_set_clientdata(client, priv);

    /* Iterate child nodes for led-device registration */
    device_for_each_child_node(&client->dev, child)
    {
        const char *label;
        struct led_device *leddev;

        ret = fwnode_property_read_string(child, "label", &label);
        if (ret)
            continue;

        leddev = devm_kzalloc(&client->dev, sizeof(*leddev), GFP_KERNEL);
        if (!leddev)
            return -ENOMEM;

        leddev->private = priv;
        leddev->brightness = 0;
        leddev->cdev.name = label;
        leddev->cdev.brightness_set_blocking = led_control;
        leddev->cdev.max_brightness = 255;

        ret = devm_led_classdev_register(&client->dev, &leddev->cdev);
        if (ret)
            return ret;

        priv->num_leds++;
        dev_info(&client->dev, "registered LED '%s'\n", label);
    }

    dev_info(&client->dev, "BlinkM: %u LEDs registered\n", priv->num_leds);
    return 0;
}

static void blinkm_remove(struct i2c_client *client)
{
    dev_info(&client->dev, "BlinkM driver removed\n");
}

static const struct i2c_device_id blinkm_id[] = {
    {"blinkm", 0},
    {}};
MODULE_DEVICE_TABLE(i2c, blinkm_id);

static const struct of_device_id blinkm_of_match[] = {
    {
        .compatible = "blinkm",
    },
    {}};
MODULE_DEVICE_TABLE(of, blinkm_of_match);

static struct i2c_driver blinkm_driver = {
    .driver = {
        .name = "blinkm",
        .of_match_table = blinkm_of_match,
        .owner = THIS_MODULE,
    },
    .probe = blinkm_probe,
    .remove = blinkm_remove,
    .id_table = blinkm_id,
};
module_i2c_driver(blinkm_driver);

MODULE_AUTHOR("Your Name <you@example.com>");
MODULE_DESCRIPTION("BlinkM I2C LED driver with 5 virtual LEDs");
MODULE_LICENSE("GPL");
