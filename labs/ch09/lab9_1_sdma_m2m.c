#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/dmaengine.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

#define SDMA_BUF_SIZE 4096

struct dma_private
{
    struct miscdevice dma_misc_device;
    struct device *dev;

    char *wbuf;
    char *rbuf;
    dma_addr_t wbuf_dma_handle; // ✅ DMA-safe physical address
    dma_addr_t rbuf_dma_handle;

    struct dma_chan *dma_m2m_chan;
    struct completion dma_m2m_ok;
};

static void dma_m2m_callback(void *data)
{
    struct dma_private *dma_priv = data;
    dev_info(dma_priv->dev, "%s: finished DMA transaction\n", __func__);
    complete(&dma_priv->dma_m2m_ok);

    if (*(dma_priv->rbuf) != *(dma_priv->wbuf))
        dev_err(dma_priv->dev, "buffer copy failed!\n");

    dev_info(dma_priv->dev, "buffer copy passed\n");
    dev_info(dma_priv->dev, "wbuf is: %s\n", dma_priv->wbuf);
    dev_info(dma_priv->dev, "rbuf is: %s\n", dma_priv->rbuf);
}

static ssize_t sdma_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    struct dma_async_tx_descriptor *dma_m2m_desc;
    struct dma_device *dma_dev;
    struct dma_private *dma_priv;
    dma_cookie_t cookie;

    dma_priv = container_of(file->private_data, struct dma_private, dma_misc_device);

    if (!dma_priv->dma_m2m_chan)
        return -ENODEV;

    dma_dev = dma_priv->dma_m2m_chan->device;

    if (copy_from_user(dma_priv->wbuf, buf, count))
        return -EFAULT;

    dev_info(dma_priv->dev, "The wbuf string is %s\n", dma_priv->wbuf);
    dev_info(dma_priv->dev, "dma_src phys: %pad\n", &dma_priv->wbuf_dma_handle);
    dev_info(dma_priv->dev, "dma_dest phys: %pad\n", &dma_priv->rbuf_dma_handle);

    dma_m2m_desc = dma_dev->device_prep_dma_memcpy(
        dma_priv->dma_m2m_chan,
        dma_priv->rbuf_dma_handle,
        dma_priv->wbuf_dma_handle,
        SDMA_BUF_SIZE,
        DMA_CTRL_ACK | DMA_PREP_INTERRUPT);

    if (!dma_m2m_desc)
    {
        dev_err(dma_priv->dev, "Failed to prepare DMA memcpy\n");
        return -EIO;
    }

    dma_m2m_desc->callback = dma_m2m_callback;
    dma_m2m_desc->callback_param = dma_priv;

    init_completion(&dma_priv->dma_m2m_ok);

    cookie = dmaengine_submit(dma_m2m_desc);
    if (dma_submit_error(cookie))
    {
        dev_err(dma_priv->dev, "Failed to submit DMA\n");
        return -EINVAL;
    }

    dma_async_issue_pending(dma_priv->dma_m2m_chan);
    wait_for_completion(&dma_priv->dma_m2m_ok);

    return count;
}

static const struct file_operations dma_fops = {
    .owner = THIS_MODULE,
    .write = sdma_write,
};

static int my_probe(struct platform_device *pdev)
{
    int retval;
    struct dma_private *dma_device;
    dma_cap_mask_t dma_m2m_mask;

    dev_info(&pdev->dev, "platform_probe enter\n");

    dma_device = devm_kzalloc(&pdev->dev, sizeof(struct dma_private), GFP_KERNEL);
    if (!dma_device)
        return -ENOMEM;

    dma_device->dev = &pdev->dev;

    // ✅ DMA-safe memory allocation
    dma_device->wbuf = dma_alloc_coherent(&pdev->dev, SDMA_BUF_SIZE,
                                          &dma_device->wbuf_dma_handle, GFP_KERNEL);
    if (!dma_device->wbuf)
    {
        dev_err(&pdev->dev, "Failed to allocate wbuf DMA buffer\n");
        return -ENOMEM;
    }

    dma_device->rbuf = dma_alloc_coherent(&pdev->dev, SDMA_BUF_SIZE,
                                          &dma_device->rbuf_dma_handle, GFP_KERNEL);
    if (!dma_device->rbuf)
    {
        dev_err(&pdev->dev, "Failed to allocate rbuf DMA buffer\n");
        return -ENOMEM;
    }

    dma_cap_zero(dma_m2m_mask);
    dma_cap_set(DMA_MEMCPY, dma_m2m_mask);
    dma_device->dma_m2m_chan = dma_request_channel(dma_m2m_mask, NULL, NULL);
    if (!dma_device->dma_m2m_chan)
    {
        dev_err(&pdev->dev, "Error requesting DMA channel\n");
        return -EINVAL;
    }

    dma_device->dma_misc_device.minor = MISC_DYNAMIC_MINOR;
    dma_device->dma_misc_device.name = "sdma_test";
    dma_device->dma_misc_device.fops = &dma_fops;

    retval = misc_register(&dma_device->dma_misc_device);
    if (retval)
    {
        dma_release_channel(dma_device->dma_m2m_chan);
        return retval;
    }

    platform_set_drvdata(pdev, dma_device);
    dev_info(&pdev->dev, "platform_probe exit\n");

    return 0;
}

static int my_remove(struct platform_device *pdev)
{
    struct dma_private *dma_device = platform_get_drvdata(pdev);
    dev_info(&pdev->dev, "platform_remove enter\n");

    misc_deregister(&dma_device->dma_misc_device);

    if (dma_device->dma_m2m_chan)
        dma_release_channel(dma_device->dma_m2m_chan);

    if (dma_device->wbuf)
        dma_free_coherent(dma_device->dev, SDMA_BUF_SIZE,
                          dma_device->wbuf, dma_device->wbuf_dma_handle);

    if (dma_device->rbuf)
        dma_free_coherent(dma_device->dev, SDMA_BUF_SIZE,
                          dma_device->rbuf, dma_device->rbuf_dma_handle);

    return 0;
}

static const struct of_device_id my_of_ids[] = {
    {.compatible = "arrow,sdma_m2m"},
    {}};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
    .probe = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "sdma_m2m",
        .of_match_table = my_of_ids,
        .owner = THIS_MODULE,
    },
};

static int __init demo_init(void)
{
    pr_info("demo_init enter\n");
    return platform_driver_register(&my_platform_driver);
}

static void __exit demo_exit(void)
{
    pr_info("demo_exit enter\n");
    platform_driver_unregister(&my_platform_driver);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mohamed.abdoun83@gmail.com");
MODULE_DESCRIPTION("SDMA memory to memory driver for Raspberry Pi 5");
