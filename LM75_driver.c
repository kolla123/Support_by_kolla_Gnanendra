#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/version.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/bcd.h>

#define LM75_REG_CONFIG       0x01
#define LM75_READ_TEMP        0x00 


dev_t dev = 0;
static struct class *dev_class;
static struct cdev my_cdev;
 
static int __init temp_driver_init(void);
static int temp_remove(struct i2c_client *client);

static int temp_open(struct inode *inode, struct file *file);
static int temp_release(struct inode *inode, struct file *file);
static ssize_t temp_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t temp_write(struct file *filp, const char *buf, size_t len, loff_t * off);

static long temp_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
//int my_ioctl(struct inode *inode,struct file *filp,unsigned int cmd,unsigned long arg);

struct temp_device_struct local_dev_ptr;

struct temp {
	struct input_dev *input_dev;
	struct i2c_client *client;
};
struct temp_device_struct{
	struct i2c_client *client;
};

static const struct i2c_device_id temp_id[] = {
	{ "lm75", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, temp_id);
 
static struct file_operations fops =
{
        .owner          	= THIS_MODULE,
        .read           	= temp_read,
        .write          	= temp_write,
        .open           	= temp_open,
       	.unlocked_ioctl 	= temp_ioctl,
        .release        	= temp_release,
};
 
static int temp_open(struct inode *inode, struct file *file)
{
        printk("Device File Opened...!!!\n");
        return 0;
}
 
static int temp_release(struct inode *inode, struct file *file)
{
        printk("Device File Closed...!!!\n");
        return 0;
}
 
static ssize_t temp_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	char buff[100] = {0};
	int ret; 
        printk("Read Function %s\n",__func__);
	ret = i2c_master_recv(local_dev_ptr.client,LM75_READ_TEMP,buff);
	if ( ret < 0) {
		printk("i2c_master_recv failed %d\n", ret);
		return ret;
	}
	printk("ret is %d\n", ret);
	printk("---buff[0] = %d  buf = %x\n", buff[0], buff[0]);
        return 0;
}
static ssize_t temp_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        printk("Write function %s\n",__func__);
 	i2c_master_send(local_dev_ptr.client,LM75_REG_CONFIG,1);
	printk("LM75_REG_CONFIG =%d\n",LM75_REG_CONFIG);
        return 0;
}

static long temp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("%s\n",__func__);

 return(0);
}

static int __init temp_init(void)
{
	printk("In %s\n", __func__);

        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "temp_Dev")) <0)
	{
                printk("Cannot allocate major number\n");
                return -1;
        }
        printk("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

        /*Creating cdev structure*/
        cdev_init(&my_cdev,&fops);
        my_cdev.owner = THIS_MODULE;
        my_cdev.ops = &fops;

        /*Adding character device to the system*/
        if((cdev_add(&my_cdev,dev,1)) < 0)
	{
            printk("Cannot add the device to the system\n");
            goto r_class;
        }

        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"temp_class")) == NULL)
	{
            printk("Cannot create the struct class\n");
            goto r_class;
        }

        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"temp_device")) == NULL)
	{
            printk("Cannot create the Device 1\n");
            goto r_device;
        }
        printk("Device Driver Inserted successfully...Done!!!\n");
	return 0;

r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
static int temp_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk("In %s\n", __func__);

	void *temp = NULL;

	printk("\nIn INIT routine of Temp_driver  %s\n", __func__);

	temp = temp_init();

    	temp = devm_kzalloc(&client->dev, sizeof(struct temp), GFP_KERNEL);
	
	local_dev_ptr.client=client;

	printk("Temp_driver probe is  successful!!!\n");
	return 0;
}
 
static int temp_remove(struct i2c_client *client)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
	printk("Device Driver Removed Successfully...Done!!!\n");
	return 0;
}

static struct i2c_driver temp_driver = {
    .driver = {
	    .name   = "lm75",
	    .owner  = THIS_MODULE,
    },
    .probe          = temp_probe,
    .remove         = temp_remove,
    .id_table       = temp_id,
};

module_i2c_driver(temp_driver); 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gnnendra.k");
MODULE_DESCRIPTION("A simple device driver for lm75 via i2c bus");
