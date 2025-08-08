#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/iio/iio.h>

struct nunchuck_accel
{
    struct i2c_client *client;
};

#define NUNCHUCK_IIO_CHAN(axis)                       \
    {                                                 \
        .type = IIO_ACCEL,                            \
        .modified = 1,                                \
        .channel2 = IIO_MOD_##axis,                   \
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW), \
    }

static const struct iio_chan_spec nunchuck_channels[] =
    {
        NUNCHUCK_IIO_CHAN(X),
        NUNCHUCK_IIO_CHAN(Y),
        NUNCHUCK_IIO_CHAN(Z),
};

static int nunchuck_read_registers(struct i2c_client *client, char *buf, int buf_size)
{

    int ret;
    mdelay(10);

    buf[0] = 0x00;
    ret = i2c_master_send(client, buf, 1);
    if (ret >= 0 && ret != 1)
        return -EIO;

    if (ret < 0)
        return ret;

    mdelay(10);

    ret = i2c_master_recv(client, buf, buf_size);
    if (ret >= 0 && ret != buf_size)
        return -EIO;
    if (ret < 0)
        return ret;

    return 0;
}

static int nunchuck_accel_read_raw(struct iio_dev *indio_dev,
                                   struct iio_chan_spec const *chan,
                                   int *val, int *val2, long mask)
{
    char buf[6];
    struct nunchuck_accel *nunchuck_accel = iio_priv(indio_dev);
    struct i2c_client *client = nunchuck_accel->client;

    // Read daat from nunchuck
    if (nunchuck_read_registers(client, buf, ARRAY_SIZE(buf)) < 0)
    {
        dev_info(&client->dev, "Error Reading the nunchuck registers. \n");
        return -EIO;
    }
    // Data needs to be written to 'val' & 'val2' is ignored
    switch (chan->channel2)
    {
    case IIO_MOD_X:
        *val = (buf[2] << 2) | ((buf[5] >> 2) & 0x3);
        break;
    case IIO_MOD_Y:
        *val = (buf[3] << 2) | ((buf[5] >> 4) & 0x3);
        break;
    case IIO_MOD_Z:
        *val = (buf[4] << 2) | ((buf[5] >> 6) & 0x3);
        break;

    default:
        return -EINVAL;
    }
    return IIO_VAL_INT;
}

static const struct iio_info nunchuck_info = {
    .read_raw = nunchuck_accel_read_raw,
};

static int nunchuck_accel_probe(struct i2c_client *client) //, const struct i2c_device_id *id
{
    int ret;
    u8 buf[2];
    struct iio_dev *indio_dev;

    // declare instance of the private struct
    struct nunchuck_accel *nunchuck_accel;
    dev_info(&client->dev, "nunchuck_accel_probe() function is called. \n");

    // Allocate the iio_dev struct
    indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*nunchuck_accel));
    if (indio_dev == NULL)
    {
        return -ENOMEM;
    }

    nunchuck_accel = iio_priv(indio_dev);
    // Associate client->dev with nunchuck private struct
    i2c_set_clientdata(client, nunchuck_accel);
    nunchuck_accel->client = client;

    indio_dev->name = "Nunchuck Accel";
    indio_dev->dev.parent = &client->dev;
    indio_dev->info = &nunchuck_info;
    indio_dev->channels = nunchuck_channels;
    indio_dev->num_channels = 3;
    indio_dev->modes = INDIO_DIRECT_MODE;

    // Nunchuck handshake
    buf[0] = 0xf0;
    buf[1] = 0x55;
    ret = i2c_master_send(client, buf, 2);
    if (ret >= 0 && ret != 2)
        return -EIO;
    if (ret < 0)
        return ret;
    ret = devm_iio_device_register(&client->dev, indio_dev);
    if (ret)
        return ret;

    dev_info(&client->dev, "nunchuck registered\n");
    return 0;
}

static int nunchuck_accel_remove(struct i2c_client *client)
{
    dev_info(&client->dev, "nunchuck_remove()\n");
    return 0;
}

// Add entries to the device Tree
static const struct of_device_id nunchuck_accel_of_match[] = {
    {.compatible = "nunchuck_accel_provider"},
    {}};

MODULE_DEVICE_TABLE(of, nunchuck_accel_of_match);

static const struct i2c_device_id nunchuck_accel_id[] = {
    {"nunchuck_accel_provider", 0},
    {}};

MODULE_DEVICE_TABLE(i2c, nunchuck_accel_id);

// Create struct i2c_driver
static struct i2c_driver nunchuck_accel_driver = {
    .driver = {
        .name = "nunchuck_accel_provider",
        .owner = THIS_MODULE,
        .of_match_table = nunchuck_accel_of_match,
    },
    .probe = nunchuck_accel_probe,
    .remove = (void *)nunchuck_accel_remove,
    .id_table = nunchuck_accel_id,
};

// register to i2c bus as driver
module_i2c_driver(nunchuck_accel_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Osman<mohamed.abdoun83@gmil.com>");
MODULE_DESCRIPTION("This is a nunchuk Accelerometer IIO framework I2C driver");
