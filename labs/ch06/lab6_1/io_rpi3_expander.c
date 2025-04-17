#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/uaccess.h>

/* Private devie structure*/

struct ioexp_dev
{
    struct i2c_client *client;
    struct miscdevice ioexp_miscdevice;
    char name[8]; // ioexpXX
};

/* User is reading data from /dev/ioexpXX*/
static ssize_t ioexp_read_file(struct file *file, char __user *userbuf, size_t count, loff_t *ppos)
{
    int expval, size;
    char buf[3];
    struct ioexp_dev *ioexp;

    ioexp = container_of(file->private_data, struct ioexp_dev, ioexp_miscdevice);

    // Store IO expander input in expval variable
    expval = i2c_smbus_read_byte(ioexp->client);
    if (expval < 0)
        return -EFAULT;

    /*
     * Convert expval int value into a char string.
     * For example, 255 int (4 bytes) = FF (2 bytes) + '\0' (1 byte) string.
     */
    size = sprintf(buf, "%02x", expval); /* size is 2 */

    /*
     * Replace NULL by \n. It is not needed to have a char array
     * ended with \0 character.
     */
    buf[size] = '\n';

    /* Send size+1 to include the \n character */
    if (*ppos == 0)
    {

        if (copy_to_user(userbuf, buf, size + 1))
        {
            pr_info("Failed to return led_value to user space\n");
            return -EFAULT;
        }
        *ppos += 1;
        return size + 1;
    }

    return 0;
}

/* Write from the terminal command line to /dev/ioexpXX, \n is added */
static ssize_t ioexp_write_file(struct file *file, const char __user *userbuf, size_t count, loff_t *ppos)
{
    int ret;
    unsigned long val;
    char buf[4];
    struct ioexp_dev *ioexp;

    ioexp = container_of(file->private_data, struct ioexp_dev, ioexp_miscdevice);

    dev_info(&ioexp->client->dev, "ioexp_write_file entered on %s\n", ioexp->name);
    dev_info(&ioexp->client->dev, "we have written %zu characters\n", count);

    if (copy_from_user(buf, userbuf, count))
    {
        dev_err(&ioexp->client->dev, "Bad copied value\n");
        return -EFAULT;
    }

    buf[count - 1] = '\0'; // replace \ with \0

    /*Convert the string to an unsinged long*/
    ret = kstrtoul(buf, 0, &val);
    if (ret)
        return -EINVAL;

    dev_info(&ioexp->client->dev, "the value is %lu\n", val);

    ret = i2c_smbus_write_byte(ioexp->client, val);
    if (ret < 0)
        dev_err(&ioexp->client->dev, "the devide is not foud\n");

    dev_info(&ioexp->client->dev, "ioexp_write_file exited on %s\n", ioexp->name);

    return count;
}

static const struct file_operations ioexp_fops = {
    .owner = THIS_MODULE,
    .read = ioexp_read_file,
    .write = ioexp_write_file,
};

/* The probe() function is called after binding */
static int ioexpo_probe(struct i2c_client *client) // removed becuase of compliation error | , const struct i2c_device_id *id
{
    static int counter = 0;
    struct ioexp_dev *ioexp;

    /* Allocate a private structure */
    ioexp = devm_kzalloc(&client->dev, sizeof(struct ioexp_dev), GFP_KERNEL);

    /* Store pointer to the device-structure in bus device context */
    i2c_set_clientdata(client, ioexp);

    /* Store pointer to the I2C client device in the private structure */
    ioexp->client = client;

    /* Initialize the misc device, ioexp is incremented after each probe call */
    sprintf(ioexp->name, "ioexp%02d", counter++);
    dev_info(&client->dev, "ioexp_probe is entered on %s\n", ioexp->name);

    ioexp->ioexp_miscdevice.name = ioexp->name;
    ioexp->ioexp_miscdevice.minor = MISC_DYNAMIC_MINOR;
    ioexp->ioexp_miscdevice.fops = &ioexp_fops;

    /* Register misc device */
    return misc_register(&ioexp->ioexp_miscdevice);
    dev_info(&client->dev, "ioexp_probe is exited on %s\n", ioexp->name);

    return 0;
}

static void ioexp_remove(struct i2c_client *client)  // changed signiture from int to void 
{
    struct ioexp_dev *ioexp;

    // Get device struct from bus dev context
    ioexp = i2c_get_clientdata(client);
    dev_info(&client->dev, "ioexp_remice is entered on %s\n", ioexp->name);

    // Deregister misc device
    misc_deregister(&ioexp->ioexp_miscdevice);
    dev_info(&client->dev, "ioexp_remove is exited on %s\n", ioexp->name);

   // return 0; //disbled becuase of exceptio du to change 
}

static const struct of_device_id ioexp_dt_ids[] = {
    {.compatible = "arrow,ioexp"},
    {},
};

MODULE_DEVICE_TABLE(of, ioexp_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
    {.name = "ioexp"},
    {},
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

static struct i2c_driver ioexp_driver = {
    .driver = {
        .name = "ioexp",
        .owner = THIS_MODULE,
        .of_match_table = ioexp_dt_ids,

    },
    .probe = ioexpo_probe,
    .remove = ioexp_remove,
    .id_table = i2c_ids,
};

module_i2c_driver(ioexp_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamed Osman <mohamed.abdoun83@gmail.com>");
MODULE_DESCRIPTION("This is a driver that controls several I2C IO expanders");