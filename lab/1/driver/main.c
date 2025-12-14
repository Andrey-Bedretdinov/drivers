#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "lab1dev"
#define MAX_BINS 40
#define BIN_WIDTH_US 5

#define IOCTL_GET_HIST_LEN _IOR('x', 1, int)
#define IOCTL_GET_HIST_BUF _IOR('x', 2, size_t[MAX_BINS])

static dev_t dev_num;
static struct cdev c_dev;
static struct class *cl;

static int stored_value = 0;
static unsigned long last_write_time = 0;

static size_t histogram[MAX_BINS] = {0};
static int hist_len = MAX_BINS;

static int dev_open(struct inode *i, struct file *f) {
  pr_info("lab1: устройство открыто\n");
  return 0;
}

static int dev_close(struct inode *i, struct file *f) {
  pr_info("lab1: устройство закрыто\n");
  return 0;
}

static ssize_t dev_write(struct file *f, const char __user *buf, size_t len,
                         loff_t *off) {
  if (len < sizeof(int))
    return -EINVAL;

  if (copy_from_user(&stored_value, buf, sizeof(int)))
    return -EFAULT;

  last_write_time = jiffies;
  return sizeof(int);
}

static ssize_t dev_read(struct file *f, char __user *buf, size_t len,
                        loff_t *off) {
  unsigned long dt = jiffies - last_write_time;
  unsigned long dt_us = jiffies_to_usecs(dt);
  int bin = dt_us / BIN_WIDTH_US;

  if (bin >= MAX_BINS)
    bin = MAX_BINS - 1;

  histogram[bin]++;

  if (copy_to_user(buf, &stored_value, sizeof(int)))
    return -EFAULT;

  return sizeof(int);
}

static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
  switch (cmd) {
  case IOCTL_GET_HIST_LEN:
    if (copy_to_user((int *)arg, &hist_len, sizeof(hist_len)))
      return -EFAULT;
    break;

  case IOCTL_GET_HIST_BUF:
    if (copy_to_user((size_t *)arg, histogram, sizeof(histogram)))
      return -EFAULT;
    break;

  default:
    return -EINVAL;
  }

  return 0;
}

static struct file_operations fops = {.owner = THIS_MODULE,
                                      .open = dev_open,
                                      .release = dev_close,
                                      .write = dev_write,
                                      .read = dev_read,
                                      .unlocked_ioctl = dev_ioctl};

static int __init lab1_init(void) {
  alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);

  cdev_init(&c_dev, &fops);
  cdev_add(&c_dev, dev_num, 1);

  cl = class_create(DEVICE_NAME);
  if (IS_ERR(cl)) {
    unregister_chrdev_region(dev_num, 1);
    return PTR_ERR(cl);
  }

  device_create(cl, NULL, dev_num, NULL, DEVICE_NAME);

  pr_info("lab1: драйвер загружен\n");
  return 0;
}

static void __exit lab1_exit(void) {
  device_destroy(cl, dev_num);
  class_destroy(cl);
  cdev_del(&c_dev);
  unregister_chrdev_region(dev_num, 1);
  pr_info("lab1: драйвер выгружен\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("andrey");
MODULE_DESCRIPTION("Lab1 char driver");

module_init(lab1_init);
module_exit(lab1_exit);
