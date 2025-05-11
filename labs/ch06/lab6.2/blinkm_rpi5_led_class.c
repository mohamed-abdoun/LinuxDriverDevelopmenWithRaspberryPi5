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
    bool run_script; /* false = command mode, true = script mode */
    struct i2c_client *client;
};

static int blinkm_send(struct i2c_client *client, u8 *buf, int len)
{
    int ret = i2c_master_send(client, buf, len);
    return (ret == len) ? 0 : (ret < 0 ? ret : -EIO);
}

static int blinkm_play(struct i2c_client *client, u8 script_id,
                       u8 repeats, u8 start_pos)
{
    u8 cmd[4] = {'p', script_id, repeats, start_pos};
    return blinkm_send(client, cmd, sizeof(cmd));
}

static int blinkm_stop(struct i2c_client *client)
{
    u8 cmd = 'o';
    return blinkm_send(client, &cmd, 1);
}

/* LED class brightness callback: only in command mode */
static int led_control(struct led_classdev *cdev, enum led_brightness value)
{
    struct led_device *leddev = container_of(cdev, struct led_device, cdev);
    struct led_priv *priv = leddev->private;

    if (priv->run_script)
        return -EPERM;
    if (value > 255)
        return -EINVAL;

    switch (cdev->name[0])
    {
    case 'r':
        return blinkm_send(priv->client, (u8[]){'n', value, 0, 0}, 4);
    case 'g':
        return blinkm_send(priv->client, (u8[]){'n', 0, value, 0}, 4);
    case 'b':
        return blinkm_send(priv->client, (u8[]){'n', 0, 0, value}, 4);
    case 'm':
        return blinkm_send(priv->client, (u8[]){'n', value, value, value}, 4);
    case 's':
        return blinkm_send(priv->client, (u8[]){'c', value, 255 - value, value}, 4);
    default:
        return -EINVAL;
    }
}

/* sysfs: mode (c = command (default), s = script) */
static ssize_t mode_show(struct device *dev,
                         struct device_attribute *attr,
                         char *buf)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct led_priv *priv = i2c_get_clientdata(client);
    dev_info(dev, "mode_show: current mode = %c\n",
             priv->run_script ? 's' : 'c');
    /* return mode if desired, else just 0 */
    return 0;
}

static ssize_t mode_store(struct device *dev,
                          struct device_attribute *attr,
                          const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct led_priv *priv = i2c_get_clientdata(client);

    if (buf[0] == 's')
    {
        priv->run_script = true;
        dev_info(dev, "mode_store: switched to script mode\n");
    }
    else if (buf[0] == 'c' || buf[0] == '\n')
    {
        priv->run_script = false;
        blinkm_stop(client);
        dev_info(dev, "mode_store: switched to command mode\n");
    }
    else
    {
        return -EINVAL;
    }
    return count;
}
static DEVICE_ATTR(mode, 0644, mode_show, mode_store);

/* sysfs: start a predefined script */
static ssize_t script_store(struct device *dev,
                            struct device_attribute *attr,
                            const char *buf, size_t count)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct led_priv *priv = i2c_get_clientdata(client);
    unsigned int script;

    if (!priv->run_script)
        return -EPERM;
    if (kstrtouint(buf, 0, &script))
        return -EINVAL;
    if (script > 0x12)
        return -EINVAL;

    blinkm_play(client, script, 0x00, 0x00);
    return count;
}
static DEVICE_ATTR(script, 0200, NULL, script_store);

static int blinkm_probe(struct i2c_client *client)
{
    struct led_priv *priv;
    struct fwnode_handle *child;
    int ret;

    priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    priv->client = client;
    priv->run_script = false; /* default to command mode */
    priv->num_leds = 0;
    i2c_set_clientdata(client, priv);

    /* create sysfs controls on the device */
    ret = sysfs_create_file(&client->dev.kobj, &dev_attr_mode.attr);
    if (ret)
        return ret;
    ret = sysfs_create_file(&client->dev.kobj, &dev_attr_script.attr);
    if (ret)
    {
        sysfs_remove_file(&client->dev.kobj, &dev_attr_mode.attr);
        return ret;
    }

    /* register virtual LEDs from device tree child nodes */
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
    }

    dev_info(&client->dev, "BlinkM: %u LEDs + mode/script ready\n",
             priv->num_leds);
    return 0;
}

static void blinkm_remove(struct i2c_client *client)
{
    sysfs_remove_file(&client->dev.kobj, &dev_attr_script.attr);
    sysfs_remove_file(&client->dev.kobj, &dev_attr_mode.attr);
    dev_info(&client->dev, "BlinkM removed\n");
}

static const struct i2c_device_id blinkm_id[] = {
    {"blinkm", 0}, {}};
MODULE_DEVICE_TABLE(i2c, blinkm_id);

static const struct of_device_id blinkm_of_match[] = {
    {.compatible = "blinkm"}, {}};
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
MODULE_DESCRIPTION("BlinkM driver with default command mode and mode/script sysfs");
MODULE_LICENSE("GPL");