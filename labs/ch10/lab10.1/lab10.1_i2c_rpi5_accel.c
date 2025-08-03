#include <linux/module.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

struct ioaccel_dev
{
    struct i2c_client *client;
    struct input_dev *input;
    struct delayed_work work;
};

#define POWER_CTL 0x2D
#define PCTL_MEASURE (1 << 3)
#define OUT_X_MSB 0x33

static void ioaccel_poll(struct work_struct *work)
{
    struct ioaccel_dev *ioaccel =
        container_of(to_delayed_work(work), struct ioaccel_dev, work);
    int val;

    val = i2c_smbus_read_byte_data(ioaccel->client, OUT_X_MSB);

    if (val > 0xC0 && val < 0xFF)
        input_report_key(ioaccel->input, KEY_1, 1);
    else
        input_report_key(ioaccel->input, KEY_1, 0);

    input_sync(ioaccel->input);

    schedule_delayed_work(&ioaccel->work, msecs_to_jiffies(50));
}

static int ioaccel_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
    struct ioaccel_dev *ioaccel;
    int error;

    dev_info(&client->dev, "ioaccel_probe()\n");

    ioaccel = devm_kzalloc(&client->dev, sizeof(*ioaccel), GFP_KERNEL);
    if (!ioaccel)
        return -ENOMEM;

    ioaccel->client = client;
    i2c_set_clientdata(client, ioaccel);

    error = i2c_smbus_write_byte_data(client, POWER_CTL, PCTL_MEASURE);
    if (error < 0)
        return error;

    ioaccel->input = devm_input_allocate_device(&client->dev);
    if (!ioaccel->input)
        return -ENOMEM;

    ioaccel->input->name = "IOACCEL keyboard";
    ioaccel->input->id.bustype = BUS_I2C;
    ioaccel->input->dev.parent = &client->dev;

    set_bit(EV_KEY, ioaccel->input->evbit);
    set_bit(KEY_1, ioaccel->input->keybit);

    error = input_register_device(ioaccel->input);
    if (error)
        return error;

    INIT_DELAYED_WORK(&ioaccel->work, ioaccel_poll);
    schedule_delayed_work(&ioaccel->work, msecs_to_jiffies(100));

    return 0;
}

static int ioaccel_remove(struct i2c_client *client)
{
    struct ioaccel_dev *ioaccel = i2c_get_clientdata(client);
    cancel_delayed_work_sync(&ioaccel->work);
    dev_info(&client->dev, "ioaccel_remove()\n");
    return 0;
}

static const struct of_device_id ioaccel_of_match[] = {
    {.compatible = "arrow,adxl345"},
    {}};
MODULE_DEVICE_TABLE(of, ioaccel_of_match);

static const struct i2c_device_id ioaccel_id[] = {
    {"adxl345", 0},
    {}};
MODULE_DEVICE_TABLE(i2c, ioaccel_id);

static struct i2c_driver ioaccel_driver = {
    .driver = {
        .name = "adxl345",
        .of_match_table = ioaccel_of_match,
    },
    .probe = ioaccel_probe, // ✅ Correct for Linux 6.6
    .remove = ioaccel_remove,
    .id_table = ioaccel_id,
};

module_i2c_driver(ioaccel_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Ibrahim <mohamed.abdoun83@gmail.com>");
MODULE_DESCRIPTION("ADXL345 Accelerometer I2C Input Driver (Workqueue polling)");
