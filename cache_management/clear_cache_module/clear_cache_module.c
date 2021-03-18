#include <linux/init.h>    // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>  // Core header for loading LKMs into the kernel
#include <linux/device.h>  // Header to support the kernel Driver Model
#include <linux/kernel.h>  // Contains types, macros, functions for the kernel
#include <linux/fs.h>      // Header for the Linux file system support
#include <linux/uaccess.h> // Required for the copy to user function

#define DEVICE_NAME "clear_cache"
#define CLASS_NAME "clrcache"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AbelChT");
MODULE_DESCRIPTION("A linux driver to clean the cache in ARMv8");
MODULE_VERSION("1.0");

extern void manual_clear_cache(void);

static int majorNumber;

static struct class *clearCacheClass = NULL;
static struct device *clearCacheDevice = NULL;

static int dev_open(struct inode *, struct file *);

static int dev_release(struct inode *, struct file *);

static struct file_operations fops =
    {
        .open = dev_open,
        .release = dev_release,
};

static int __init dev_init(void)
{
    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0)
    {
        printk(KERN_ALERT "clear_cache: failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "clear_cache: registered correctly with major number %d\n",
           majorNumber);

    // Register the device class
    clearCacheClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(clearCacheClass))
    { // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "clear_cache: failed to register device class\n");
        return PTR_ERR(clearCacheClass); // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "clear_cache: device class registered correctly\n");

    // Register the device driver
    clearCacheDevice = device_create(clearCacheClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(clearCacheDevice))
    {                                   // Clean up if there is an error
        class_destroy(clearCacheClass); // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "clear_cache: failed to create the device\n");
        return PTR_ERR(clearCacheDevice);
    }
    printk(KERN_INFO "clear_cache: device class created correctly\n");
    return 0;
}

static void __exit dev_exit(void)
{
    device_destroy(clearCacheClass, MKDEV(majorNumber, 0)); // remove the device
    class_unregister(clearCacheClass);                      // unregister the device class
    class_destroy(clearCacheClass);                         // remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);            // unregister the major number
    printk(KERN_INFO "clear_cache: Removed module\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    // printk(KERN_INFO "clear_cache: Device has been opened \n");
    manual_clear_cache();
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    // printk(KERN_INFO "clear_cache: Device successfully closed\n");
    return 0;
}

module_init(dev_init);
module_exit(dev_exit);