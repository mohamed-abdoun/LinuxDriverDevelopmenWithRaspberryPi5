//
// Created by mohamed on 3/24/25.
//
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/uio_driver.h>

static int my_probe(struct platform_device *pdev)
{
    int i, ret_val;
    struct device *dev = &pdev->dev;
    struct uio_info *the_uio_info;

    dev_info(dev, "plaform_probe enter\n");

    /* Allocate memory that will automatically be freed when the device is detached*/
    the_uio_info = devm_kzalloc(dev, sizeof(*the_uio_info), GFP_KERNEL);
    if (!the_uio_info)
    {
        dev_err(dev, "kzalloc failed to allocate uio_info struct\n");
        return -ENOMEM;
    }
    /* Initialize uio_info struct uio_mem array */
    the_uio_info->name = "led_uio";
    the_uio_info->version = "1.0";

    /* Register gpio, rio, and pads resources */
    for (i = 0; i < 3; i++)
    {
        struct resource *r;
        void __iomem *addr;

        r = platform_get_resource(pdev, IORESOURCE_MEM, i);
        if (!r)
        {
            dev_err(dev, "failed to get platform_resource %d\n", i);
            return -EINVAL;
        }
        addr = devm_ioremap(dev, r->start, resource_size(r));
        if (!addr)
        {
            dev_err(dev, "failed to map platform mem resource %d\n", i);
            return -EINVAL;
        }

        /* Physical address needed for the kernel user mapping */
        the_uio_info->mem[i].memtype = UIO_MEM_PHYS;
        the_uio_info->mem[i].addr = r->start;
        the_uio_info->mem[i].size = resource_size(r);
        the_uio_info->mem[i].name = "demo_uio_driver_hw_region";
        /* Virtual address for internal driver use */
        the_uio_info->mem[i].internal_addr = addr;
    }

    /* Register the uio device */
    ret_val = uio_register_device(dev, the_uio_info);
    if (ret_val)
    {
        dev_err(dev, "failed to register uio device, %d\n", ret_val);
        return ret_val;
    }

    platform_set_drvdata(pdev, the_uio_info);
    return 0;
}

static int my_remove(struct platform_device *pdev)
{
    struct uio_info *info = platform_get_drvdata(pdev);
    uio_unregister_device(info);
    dev_info(&pdev->dev, "platform_remove: uio_unregister_device exit successfully\n");
    return 0;
}

static const struct of_device_id my_of_ids[] = {
    {.compatible = "arrow,UIO"},
    {}};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
    .probe = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "led_uio_driver",
        .of_match_table = my_of_ids,
        .owner = THIS_MODULE,
    }};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("UIO platform driver");
MODULE_AUTHOR("Mohamed Osman <mohamed.abdoun83@gmail.com>");