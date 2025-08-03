#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/input.h>

/* Create private structure */
struct nunchuk_dev {
	struct input_dev	*input;
	struct i2c_client	*client;
};

static int nunchuk_read_registers(struct i2c_client *client, u8 *buf, int buf_size)
{
	int status;

	mdelay(10);

	buf[0] = 0x00;
	status = i2c_master_send(client, buf, 1);
	if (status >= 0 && status != 1)
		return -EIO;
	if (status < 0)
		return status;
	

	mdelay(10);
	status = i2c_master_recv(client, buf, buf_size);
	if (status >= 0 && status != buf_size)
		return -EIO;
	if (status < 0)
		return status;

	return 0;
}

/*
 * Poll handler function reads the hardware, queue
 * events to be reported (input_report_*) and flushes
 * the queued events (input_sync)
 */
static void nunchuk_poll(struct input_dev *input)
{
	struct nunchuk_dev *nunchuk = input_get_drvdata(input);
	struct i2c_client *client = nunchuk->client;
	int joy_x, joy_y, z_button, c_button, accel_x, accel_y, accel_z;
	int ret;
	u8 buf[6];

	/*
	 * Recover the global nunchuk struct and from it the client address
	 * to establish and I2C transaction with the nunchuk device.
	 */
	ret = nunchuk_read_registers(client, buf, ARRAY_SIZE(buf));
	if (ret < 0) {
		dev_err(&client->dev, "Error reading the nunchuk registers\n");
		return;
	}

	joy_x = buf[0];
	joy_y = buf[1];

	/* bit 0 indicates if Z button is pressed */
	z_button = (buf[5] & BIT(0) ? 0: 1);
	/* bit 1 indicates if C button is pressed */
	c_button = (buf[5] & BIT(1) ? 0: 1);

	accel_x = (buf[2] << 2) | ((buf[5] >> 2) & 0x3);
	accel_y = (buf[3] << 2) | ((buf[5] >> 4) & 0x3);
	accel_z = (buf[4] << 2) | ((buf[5] >> 6) & 0x3);

	/* Report events to the input system */
	input_report_abs(input, ABS_X, joy_x);
	input_report_abs(input, ABS_Y, joy_y);

	input_event(input, EV_KEY, BTN_Z, z_button);
	input_event(input, EV_KEY, BTN_C, c_button);

	input_report_abs(input, ABS_RX, accel_x);
	input_report_abs(input, ABS_RY, accel_y);
	input_report_abs(input, ABS_RZ, accel_z);

	/*
	 * Tell those who receive the events that
	 * a complete report has been sent
	 */
	input_sync(input);
}

static int nunchuk_probe(struct i2c_client *client)
{
	struct nunchuk_dev *nunchuk;
	u8 buf[2];
	int ret;


	dev_info(&client->dev, "nunchuk_probe function is called\n");

	/* Allocate private structure for new device */
	nunchuk = devm_kzalloc(&client->dev, sizeof(*nunchuk), GFP_KERNEL);
	if (!nunchuk) {
		dev_err(&client->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	/* Associate client->dev with nunchuk private structure */
	i2c_set_clientdata(client, nunchuk);

	/* Allocate struct input device */
	nunchuk->input = devm_input_allocate_device(&client->dev);
	if (!nunchuk->input) {
		dev_err(&client->dev, "unable to allocate input device\n");
		return -ENOMEM;
	}

	/* Store the client device in the global structure */
	nunchuk->client = client;

	/* Initialize input polled device */
	nunchuk->input->name = "WII Nunchuk";
	nunchuk->input->id.bustype = BUS_I2C;
	input_setup_polling(nunchuk->input, nunchuk_poll);
	input_set_poll_interval(nunchuk->input, 50);

	/* Set EV_KEY type events and from those BTN_C and BTN_Z event codes */
	set_bit(EV_KEY, nunchuk->input->evbit);
	set_bit(BTN_C, nunchuk->input->keybit); /* buttons */
	set_bit(BTN_Z, nunchuk->input->keybit);

	/*
	 * Set EV_ABS type events and from those
	 * ABS_X, ABS_Y, ABS_RX, ABS_RY, and ABS_RZ event codes
	 */
	set_bit(EV_ABS, nunchuk->input->evbit);
	set_bit(ABS_X, nunchuk->input->absbit); /* joystick */
	set_bit(ABS_Y, nunchuk->input->absbit);
	set_bit(ABS_RX, nunchuk->input->absbit); /* accelerometer */
	set_bit(ABS_RY, nunchuk->input->absbit);
	set_bit(ABS_RZ, nunchuk->input->absbit);

	/*
	 * Fill additional fields in the input_dev struct for
	 * each absolute axis nunchuk has
	 */
	input_set_abs_params(nunchuk->input, ABS_X, 0x00, 0xff, 0, 0);
	input_set_abs_params(nunchuk->input, ABS_Y, 0x00, 0xff, 0, 0);

	input_set_abs_params(nunchuk->input, ABS_RX, 0x00, 0x3ff, 0, 0);
	input_set_abs_params(nunchuk->input, ABS_RY, 0x00, 0x3ff, 0, 0);
	input_set_abs_params(nunchuk->input, ABS_RZ, 0x00, 0x3ff, 0, 0);

	/* Nunchuk handshake */
	buf[0] = 0xf0;
	buf[1] = 0x55;
	ret = i2c_master_send(client, buf, 2);
	if (ret >= 0 && ret != 2)
		return -EIO;
	if (ret < 0)
		return ret;

	udelay(1);
	buf[0] = 0xfb;
	buf[1] = 0x00;
	ret = i2c_master_send(client, buf, 1);
	if (ret >= 0 && ret != 1)
		return -EIO;
	if (ret < 0)
		return ret;

	/* Finally, register the input device */
	input_set_drvdata(nunchuk->input, nunchuk);
	return input_register_device(nunchuk->input);
}

static void nunchuk_remove(struct i2c_client *client)
{
	struct nunchuk_dev *nunchuk = i2c_get_clientdata(client);

	input_unregister_device(nunchuk->input);
	dev_info(&client->dev, "nunchuk_remove()\n");
}

/* Add entries to Device Tree */
static const struct of_device_id nunchuk_of_match[] = {
	{ .compatible = "nunchuk" },
	{ },
};
MODULE_DEVICE_TABLE(of, nunchuk_of_match);

static const struct i2c_device_id nunchuk_id[] = {
	{ "nunchuk", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, nunchuk_id);

/* Create struct i2c_driver */
static struct i2c_driver nunchuk_driver = {
	.driver = {
		.name		= "nunchuk",
		.owner		= THIS_MODULE,
		.of_match_table	= nunchuk_of_match,
	},
	.probe	= nunchuk_probe,
	.remove = nunchuk_remove,
	.id_table = nunchuk_id,
};

/* Register to I2C bus as driver */
module_i2c_driver(nunchuk_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a Nunchuk Wii I2C driver");
