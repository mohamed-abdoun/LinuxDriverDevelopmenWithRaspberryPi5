#include <linux/module.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/input-polldev.h>
#include <linux/platform_device.h>

// Create private struct
struct nunchuck_dev
{
    struct input_polled_dev *polled_input;
    // declare pointers to IIO channels of the provider
    struct iio_channel *accel_x, *accel_y, *acel_z;
};

/**
 * Poll handler function reads the hardware, queues events to be reported (input_report_*)
 * and fulshes the queued events (inputs_sync)
 */

static void nunchuck_poll(struct input_polled_dev *polled_input)
{
    int accel_x, accel_y, accel_z;
    struct nunchuck_dev *nunchuck;
    int ret;

    nunchuck = polled_input->private;
    // ################## X ######################
    // Read IIO accel_x event to the input stream
    ret = iio_read_channel_raw(nunchuck->accel_x, &accel_x);
    if (unlikely(ret < 0))
        return;
    // Report ABS_RX event to the input system
    input_report_abs(polled_input->input, ABS_RX, accel_x);
    // ################## Y ######################
    ///* Read IIO "accel_y" channel raw value from the provider device
    ret = iio_read_channel_raw(nunchuck->accel_y, &accel_y);
    if (unlikely(ret < 0))
        return;
    // report ABS_RY event to the input sysyem
    input_report_abs(polled_input->input, ABS_RY, accel_y);
    // ################## z ######################
    ///* Read IIO "accel_y" channel raw value from the provider device
    ret = iio_read_channel_raw(nunchuck->accel_z, &accel_z);
    if (unlikely(ret < 0))
        return;
    // report ABS_RY event to the input sysyem
    input_report_abs(polled_input->input, ABS_RZ, accel_z);

    // Tell those who receive the event that a complete report has been sent
    input_sync(polled_input->input);
}

static int nunchuck_probe(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;
    enum iio_chan_type type;

    // Declare a pointer to the private struct
    struct nunchuck_dev *nunchuck;

    // declare pointer to input_dev and input_polled_dev structs
    struct input_polled_dev *polled_device;
    struct input_dev *input;

    dev_info(dev, "nunchuck_probe() function is called.\n");

    // allocate private struct for nunchuck device
    nunchuck = devm_kzalloc(dev, sizeof(*nunchuck), GFP_KERNEL);
    if (nunchuck == NULL)
        return -ENOMEM;

    // ################## X ######################
    // get pointer to channel "accel_x" of the provider device
    nunchuck->accel_x = devm_iio_channel_get(dev, "accel_x");
    if (IS_ERR(nunchuck->accel_x))
        return PTR_ERR(nunchuck->accel_x);
    if (!nunchuch->accel_x->indio_dev)
        return -ENXIO;
    // Get the type of the ""accel_x" channel
    ret = iio_get_channel_type(nunchuck->accel_x, &type);
    if (ret < 0)
        return ret;
    if (type != IIO_ACCEL)
    {
        dev_err(dev, "not accelerometer channel.\n");
        return -EINVAL;
    }

    // ################## Y ######################
    // get pointer to channel "accel_y" of the provider device
    nunchuck->accel_y = devm_iio_channel_get(dev, "accel_y");
    if (IS_ERR(nunchuck->accel_y))
        return PTR_ERR(nunchuck->accel_y);
    if (!nunchuck->accel_y->indio_dev)
        return -ENXIO;
    // get type of "accel_y" channel
    ret = iio_get_channel_type(nunchuck->accel_y, &type);
    if (ret < 0)
        return ret;
    if (type != IIO_ACCEL)
    {
        dev_err(dev, "not accel channel %d\n", type);
        return -EINVAL;
    }
    // ################## Z ######################
    // get pointer to channel "accel_z" of the provider device
    nunchuck->accel_z = devm_iio_channel_get(dev, "accel_z");
    if (IS_ERR(nunchuck->accel_z))
        return PTR_ERR(nunchuck->accel_z);
    if (!nunchuck->accel_z->indio_dev)
        return -ENXIO;
    // get type of "accel_z" channel
    ret = iio_get_channel_type(nunchuck->accel_z, &type);
    if (ret < 0)
        return ret;
    if (type != IIO_ACCEL)
    {
        dev_err(dev, "not accel channel %d\n", type);
        return -EINVAL;
    }

    // Allocate struct input_polled_dev
    polled_device = devm_input_allocate_polled_device(dev);
    if (!polled_device)
    {
        dev_err(dev, "unable to allocate input device\n");
        return -ENOMEM;
    }
    // initiatlize the polled input device to recover nunchuck in poll() function
    polled_device->private = nunchuck;
    // fill in the poll interval
    polled_device->poll_interval = 50;

    polled_device->input->name = "WII accel consumer";
    polled_device->input->id.bustype = BUS_HOST;

    // store the polled device in the global struct to recover it in the remove() function
    nunchuck->polled_input = polled_device;
    input = polled_device->input;

    // To revoer nunchuck struct from remove() function
    platform_set_drvdata(pdev, nunchuck);

    // Set EV_ABS type events and from those ABS_X,ABS_Y,ABS_RX,ABS_RY and ABS_RZ event codes
    set_bit(EV_ABS, input->evbit);
    set_bit(ABS_RX, input->absbit); // accelerometer
    set_bit(ABS_RY, input->absbit);
    set_bit(ABS_RZ, input->absbit);

    // Fill additional feilds in the input_dev struct for each absolute axis nunchuck has
    input_set_abs_params(input, ABS_RX, 0x00, 0x3ff, 0, 0);
    input_set_abs_params(input, ABS_RY, 0x00, 0x3ff, 0, 0);
    input_set_abs_params(input, ABS_RZ, 0x00, 0x3ff, 0, 0);

    // finally ,register the input device
    ret = input_register_polled_device(nunchuck->polled_input);
    if (ret < 0)
    {
        return ret;
    }

    return 0;
}

static int nunchuck_remove(struct platform_device *pdev)
{
    struct nunchuck_dev *nunchuck = platform_get_drvdata(pdev);
    input_unregister_polled_device(nunchuck->polled_input);
    dev_info(&pdev->dev, "nunchuck_remove()\n");

    return 0;
}

// Add entries to the device tree , declare a list of devices supported by the driver
static const struct of_device_id nunchuck_of_ids[] = {
    {.compatible = "nunchuck_consumer"},
    {},
};

MODULE_DEVICE_TABLE(of, nunchuck_of_ids);

// define a platform driver struct
static struct platform_driver nunchuck_platform_driver = {
    .probe = nunchuck_probe,
    .remove = nunchuck_remove,
    .driver = {
        .name = "nunchuck_consumer",
        .of_match_table = nunchuck_of_ids,
        .owner = THIS_MODULE,
    }};

// register the platform driver
module_platform_driver(nunchuck_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Osman<mohamed.abdoun83@gmail.com>");
MODULE_DESCRIPTION("This is a Nunchuk consumer platform driver");