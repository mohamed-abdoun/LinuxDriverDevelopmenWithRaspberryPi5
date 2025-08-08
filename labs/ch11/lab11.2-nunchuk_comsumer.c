#include <linux/module.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/workqueue.h>  // For workqueue support

struct nunchuck_dev {
    struct input_dev *input;
    struct iio_channel *accel_x, *accel_y, *accel_z;
    struct timer_list poll_timer;  // Timer for periodic polling
    struct work_struct poll_work; // Workqueue for I/O operations
};

/**
 * Read the accelerometer data and report the events
 */
static void nunchuck_poll_work(struct work_struct *work)
{
    struct nunchuck_dev *nunchuck = container_of(work, struct nunchuck_dev, poll_work);
    int accel_x, accel_y, accel_z;
    int ret;

    dev_dbg(&nunchuck->input->dev, "nunchuck_poll_work: Reading accelerometer data...\n");

    // Read accelerometer channels
    ret = iio_read_channel_raw(nunchuck->accel_x, &accel_x);
    if (ret < 0) {
        dev_err(&nunchuck->input->dev, "Failed to read accel_x: %d\n", ret);
        return;
    }
    dev_dbg(&nunchuck->input->dev, "Read accel_x: %d\n", accel_x);

    ret = iio_read_channel_raw(nunchuck->accel_y, &accel_y);
    if (ret < 0) {
        dev_err(&nunchuck->input->dev, "Failed to read accel_y: %d\n", ret);
        return;
    }
    dev_dbg(&nunchuck->input->dev, "Read accel_y: %d\n", accel_y);

    ret = iio_read_channel_raw(nunchuck->accel_z, &accel_z);
    if (ret < 0) {
        dev_err(&nunchuck->input->dev, "Failed to read accel_z: %d\n", ret);
        return;
    }
    dev_dbg(&nunchuck->input->dev, "Read accel_z: %d\n", accel_z);

    // Report the data as ABS events
    input_event(nunchuck->input, EV_ABS, ABS_X, accel_x);
    input_event(nunchuck->input, EV_ABS, ABS_Y, accel_y);
    input_event(nunchuck->input, EV_ABS, ABS_Z, accel_z);
    input_sync(nunchuck->input);  // Sync the events
    dev_dbg(&nunchuck->input->dev, "Reported accelerometer data to input subsystem.\n");
}

/**
 * Timer handler for periodic polling
 */
static void nunchuck_timer_func(struct timer_list *t)
{
    struct nunchuck_dev *nunchuck = from_timer(nunchuck, t, poll_timer);

    dev_dbg(&nunchuck->input->dev, "nunchuck_timer_func: Timer triggered, queuing work...\n");

    // Queue the work to be done outside the atomic context
    queue_work(system_wq, &nunchuck->poll_work);

    // Reschedule the timer for the next polling cycle
    mod_timer(&nunchuck->poll_timer, jiffies + msecs_to_jiffies(50));
    dev_dbg(&nunchuck->input->dev, "nunchuck_timer_func: Timer rescheduled for next poll.\n");
}

static int nunchuck_probe(struct platform_device *pdev)
{
    int ret;
    struct nunchuck_dev *nunchuck;
    struct input_dev *input_dev;
    enum iio_chan_type type;

    dev_info(&pdev->dev, "nunchuck_probe: Initializing Nunchuck consumer driver...\n");

    // Allocate and initialize the private data structure
    nunchuck = devm_kzalloc(&pdev->dev, sizeof(*nunchuck), GFP_KERNEL);
    if (!nunchuck)
        return -ENOMEM;

    // Allocate input device
    input_dev = devm_input_allocate_device(&pdev->dev);
    if (!input_dev)
        return -ENOMEM;

    dev_dbg(&pdev->dev, "nunchuck_probe: Input device allocated.\n");

    // Set up the input device
    input_dev->name = "Nunchuck Accelerometer";
    input_dev->id.bustype = BUS_HOST;
    set_bit(EV_ABS, input_dev->evbit);
    set_bit(ABS_X, input_dev->absbit);
    set_bit(ABS_Y, input_dev->absbit);
    set_bit(ABS_Z, input_dev->absbit);

    input_set_abs_params(input_dev, ABS_X, -1024, 1024, 0, 0);
    input_set_abs_params(input_dev, ABS_Y, -1024, 1024, 0, 0);
    input_set_abs_params(input_dev, ABS_Z, -1024, 1024, 0, 0);

    // Assign the input device to the private structure
    nunchuck->input = input_dev;
    platform_set_drvdata(pdev, nunchuck);

    dev_dbg(&pdev->dev, "nunchuck_probe: Input device parameters set.\n");

    // Initialize IIO channels
    nunchuck->accel_x = devm_iio_channel_get(&pdev->dev, "accel_x");
    if (IS_ERR(nunchuck->accel_x)) {
        dev_err(&pdev->dev, "Failed to get accel_x channel: %ld\n", PTR_ERR(nunchuck->accel_x));
        return PTR_ERR(nunchuck->accel_x);
    }
    dev_dbg(&pdev->dev, "nunchuck_probe: accel_x channel obtained.\n");

    nunchuck->accel_y = devm_iio_channel_get(&pdev->dev, "accel_y");
    if (IS_ERR(nunchuck->accel_y)) {
        dev_err(&pdev->dev, "Failed to get accel_y channel: %ld\n", PTR_ERR(nunchuck->accel_y));
        return PTR_ERR(nunchuck->accel_y);
    }
    dev_dbg(&pdev->dev, "nunchuck_probe: accel_y channel obtained.\n");

    nunchuck->accel_z = devm_iio_channel_get(&pdev->dev, "accel_z");
    if (IS_ERR(nunchuck->accel_z)) {
        dev_err(&pdev->dev, "Failed to get accel_z channel: %ld\n", PTR_ERR(nunchuck->accel_z));
        return PTR_ERR(nunchuck->accel_z);
    }
    dev_dbg(&pdev->dev, "nunchuck_probe: accel_z channel obtained.\n");

    // Initialize workqueue
    INIT_WORK(&nunchuck->poll_work, nunchuck_poll_work);
    dev_dbg(&pdev->dev, "nunchuck_probe: Workqueue initialized.\n");

    // Set up the timer for polling
    timer_setup(&nunchuck->poll_timer, nunchuck_timer_func, 0);
    mod_timer(&nunchuck->poll_timer, jiffies + msecs_to_jiffies(50));
    dev_dbg(&pdev->dev, "nunchuck_probe: Timer set up and started.\n");

    // Register the input device
    ret = input_register_device(input_dev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to register input device: %d\n", ret);
        return ret;
    }
    dev_info(&pdev->dev, "nunchuck_probe: Nunchuck input device registered successfully.\n");

    return 0;
}

static int nunchuck_remove(struct platform_device *pdev)
{
    struct nunchuck_dev *nunchuck = platform_get_drvdata(pdev);

    // Remove the polling timer and workqueue
    del_timer_sync(&nunchuck->poll_timer);
    cancel_work_sync(&nunchuck->poll_work);
    dev_dbg(&pdev->dev, "nunchuck_remove: Timer and workqueue cleaned up.\n");

    // Unregister the input device
    input_unregister_device(nunchuck->input);
    dev_info(&pdev->dev, "nunchuck_remove: Nunchuck input device unregistered.\n");

    return 0;
}

static const struct of_device_id nunchuck_of_ids[] = {
    { .compatible = "nunchuck_consumer" },
    {},
};
MODULE_DEVICE_TABLE(of, nunchuck_of_ids);

static struct platform_driver nunchuck_platform_driver = {
    .probe = nunchuck_probe,
    .remove = nunchuck_remove,
    .driver = {
        .name = "nunchuck_consumer",
        .of_match_table = nunchuck_of_ids,
        .owner = THIS_MODULE,
    },
};

module_platform_driver(nunchuck_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Osman");
MODULE_DESCRIPTION("Nunchuck consumer driver using event-driven input");
