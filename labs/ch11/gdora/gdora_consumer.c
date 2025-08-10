#include <linux/module.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/platform_device.h>


/* Create private structure */
struct nunchuk_dev {
	struct input_dev		*input;
	struct iio_channel		*accel_x;
	struct iio_channel		*accel_y;
	struct iio_channel		*accel_z;
};

/*
 * The poll handler function reads the hanrdware,
 * queues events to be reported (input_report_*)
 * and flushes the queueed events (input_sync)
 */
static void nunchuk_poll(struct input_dev *input)
{
	int accel_x, accel_y, accel_z;
	struct nunchuk_dev *nunchuk;
	int ret;

	nunchuk = input_get_drvdata(input);

	ret = iio_read_channel_raw(nunchuk->accel_x, &accel_x);
	if (unlikely(ret < 0))
		return;
	input_report_abs(input, ABS_RX, accel_x);

	ret = iio_read_channel_raw(nunchuk->accel_y, &accel_y);
	if (unlikely(ret < 0))
		return;
	input_report_abs(input, ABS_RY, accel_y);

	ret = iio_read_channel_raw(nunchuk->accel_z, &accel_z);
	if (unlikely(ret < 0))
		return;
	input_report_abs(input, ABS_RZ, accel_z);

	input_sync(input);
}

static int nunchuk_setup_iio_accel(struct iio_channel **paccel,
				   struct device *dev, char *name)
{
	struct iio_channel *accel;
	enum iio_chan_type type;
	int ret;

	accel = devm_iio_channel_get(dev, "accel_x");
	if (IS_ERR(accel))
		return PTR_ERR(accel);
	
	if (!accel->indio_dev)
		return -ENXIO;

	ret = iio_get_channel_type(accel, &type);
	if (ret < 0)
		return ret;
	
	if (type != IIO_ACCEL) {
		dev_err(dev, "not accelerometer channel %d\n", type);
		return -EINVAL;
	}

	*paccel = accel;
	return 0;
}

static int nunchuk_probe(struct platform_device *pdev)
{
	struct nunchuk_dev *nunchuk;
	int ret;


	dev_info(&pdev->dev, "nunchuck_probe() function is called\n");

	nunchuk = devm_kzalloc(&pdev->dev, sizeof(*nunchuk), GFP_KERNEL);
	if (!nunchuk)
		return -ENOMEM;
	
	ret = nunchuk_setup_iio_accel(&nunchuk->accel_x, &pdev->dev, "accel_x");
	if (ret)
		return ret;

	ret = nunchuk_setup_iio_accel(&nunchuk->accel_y, &pdev->dev, "accel_y");
	if (ret)
		return ret;

	ret = nunchuk_setup_iio_accel(&nunchuk->accel_z, &pdev->dev, "accel_z");
	if (ret)
		return ret;

	nunchuk->input = devm_input_allocate_device(&pdev->dev);
	if (!nunchuk->input) {
		dev_err(&pdev->dev, "unable to allocate input device\n");
		return -ENOMEM;
	}

	/* Initialize the input device */
	nunchuk->input->name = "WII accel consumer";
	nunchuk->input->id.bustype = BUS_HOST;
	input_setup_polling(nunchuk->input, nunchuk_poll);
	input_set_poll_interval(nunchuk->input, 50);
	input_set_drvdata(nunchuk->input, nunchuk);

	/* Set event types and ranges */
	set_bit(EV_ABS, nunchuk->input->evbit);
	set_bit(ABS_RX, nunchuk->input->absbit);
	set_bit(ABS_RY, nunchuk->input->absbit);
	set_bit(ABS_RZ, nunchuk->input->absbit);
	input_set_abs_params(nunchuk->input, ABS_RX, 0x00, 0x3ff, 0, 0);
	input_set_abs_params(nunchuk->input, ABS_RY, 0x00, 0x3ff, 0, 0);
	input_set_abs_params(nunchuk->input, ABS_RZ, 0x00, 0x3ff, 0, 0);

	platform_set_drvdata(pdev, nunchuk);
	return input_register_device(nunchuk->input);
}

static int nunchuk_remove(struct platform_device *pdev)
{
	struct nunchuk_dev *nunchuk = platform_get_drvdata(pdev);

	input_unregister_device(nunchuk->input);
	dev_info(&pdev->dev, "nunchuk_remove()\n");
	return 0;
}

/* Add entries to the Device Tree */
/* Declare a list of devices supported by the driver */
static const struct of_device_id nunchuk_of_ids[] = {
	{ .compatible = "nunchuk_consumer" },
	{ },
};
MODULE_DEVICE_TABLE(of, nunchuk_of_ids);

/* Define a platforrm driver structure */
static struct platform_driver nunchuk_platform_driver = {
	.probe		= nunchuk_probe,
	.remove		= nunchuk_remove,
	.driver		= {
		.name		= "nunchuk_consumer",
		.of_match_table	= nunchuk_of_ids,
		.owner		= THIS_MODULE,
	},
};


/* Register the platform driver */
module_platform_driver(nunchuk_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a Nunchuk consumer platform driver");
