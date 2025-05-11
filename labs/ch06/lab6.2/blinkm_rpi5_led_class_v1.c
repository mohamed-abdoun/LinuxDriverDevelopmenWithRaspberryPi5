#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/slab.h>

struct blinkm_priv
{
    struct i2c_client *client;
    struct led_classdev cdev;
};

static int blinkm_send(struct i2c_client *client, u8 *buf, int len)
{
    int ret = i2c_master_send(client, buf, len);
    return (ret == len) ? 0 : (ret < 0 ? ret : -EIO);
}

static int blinkm_set_rgb(struct i2c_client *client, u8 R, u8 G, u8 B)
{
    u8 cmd[4] = {'n', R, G, B};
    return blinkm_send(client, cmd, sizeof(cmd));
}

static int __maybe_unused blinkm_set_fade(struct i2c_client *client, u8 R, u8 G, u8 B)
{
    u8 cmd[4] = {'c', R, G, B};
    return blinkm_send(client, cmd, sizeof(cmd));
}

static int __maybe_unused blinkm_stop_script(struct i2c_client *client)
{
    u8 cmd = 'o';
    return blinkm_send(client, &cmd, 1);
}

static int blinkm_brightness_set(struct led_classdev *cdev,
                                 enum led_brightness value)
{
    struct blinkm_priv *priv = container_of(cdev,
                                            struct blinkm_priv,
                                            cdev);
    return blinkm_set_rgb(priv->client, value, value, value);
}

/* Use legacy probe signature (single parameter) */
static int blinkm_probe(struct i2c_client *client)
{
    struct blinkm_priv *priv;
    int ret;

    priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    priv->client = client;
    priv->cdev.name = "blinkm0";
    priv->cdev.brightness_set_blocking = blinkm_brightness_set;
    priv->cdev.max_brightness = 255;

    i2c_set_clientdata(client, priv);

    ret = devm_led_classdev_register(&client->dev, &priv->cdev);
    if (ret)
    {
        dev_err(&client->dev, "failed to register LED\n");
        return ret;
    }

    dev_info(&client->dev, "BlinkM LED registered as %s\n", priv->cdev.name);
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
MODULE_DESCRIPTION("BlinkM I2C LED driver");
MODULE_LICENSE("GPL");
