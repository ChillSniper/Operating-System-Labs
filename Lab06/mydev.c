#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mydev"
#define BUF_LEN      50

static int major_num;
static int device_open_count = 0;
static char device_buffer[BUF_LEN];

/* open 接口 */
static int mydev_open(struct inode *inode, struct file *file)
{
    if (device_open_count)
        return -EBUSY;
    device_open_count++;
    try_module_get(THIS_MODULE);
    return 0;
}

/* release 接口 */
static int mydev_release(struct inode *inode, struct file *file)
{
    device_open_count--;
    module_put(THIS_MODULE);
    return 0;
}

/* read 接口 */
static ssize_t mydev_read(struct file *file, char __user *buf,
                          size_t count, loff_t *ppos)
{
    size_t to_read = min(count, (size_t)BUF_LEN);
    if (copy_to_user(buf, device_buffer, to_read))
        return -EFAULT;
    return to_read;
}

/* write 接口 */
static ssize_t mydev_write(struct file *file,
                           const char __user *buf,
                           size_t count, loff_t *ppos)
{
    size_t to_write = min(count, (size_t)(BUF_LEN - 1));
    if (copy_from_user(device_buffer, buf, to_write))
        return -EFAULT;
    device_buffer[to_write] = '\0';
    return to_write;
}

/* file_operations 结构体 */
static const struct file_operations mydev_fops = {
    .owner   = THIS_MODULE,
    .open    = mydev_open,
    .release = mydev_release,
    .read    = mydev_read,
    .write   = mydev_write,
};

/* 模块加载函数 */
static int __init mydev_init(void)
{
    major_num = register_chrdev(0, DEVICE_NAME, &mydev_fops);
    if (major_num < 0) {
        pr_err("register_chrdev failed: %d\n", major_num);
        return major_num;
    }
    pr_info("Registered '%s' with major %d\n", DEVICE_NAME, major_num);
    return 0;
}

/* 模块卸载函数 */
static void __exit mydev_exit(void)
{
    unregister_chrdev(major_num, DEVICE_NAME);
    pr_info("Unregistered '%s'\n", DEVICE_NAME);
}

module_init(mydev_init);
module_exit(mydev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple character device driver");
