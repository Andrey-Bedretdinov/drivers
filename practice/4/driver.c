#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define BUF_SIZE 256

#define IOCTL_CLEAR _IO('q', 1)  // очистка буфера
#define IOCTL_HASDATA _IOR('q', 2, int) // узнать, есть ли данные

static char gbuf[BUF_SIZE]; // глобальный буфер
static int gbuf_len = 0;  // сколько данных в буфере

static dev_t dev_num; // номер устройства
static struct cdev my_cdev;
static struct class *my_class;

// open
static int drv_open(struct inode *inode, struct file *file) {
  pr_info("устройство открыто\n");
  return 0;
}

// release
static int drv_release(struct inode *inode, struct file *file) {
  pr_info("устройство закрыто\n");
  return 0;
}

// read
static ssize_t drv_read(struct file *file, char __user *buf, size_t size,
                        loff_t *off) {
  if (*off > 0)
    return 0;

  if (gbuf_len == 0)
    return 0;

  if (copy_to_user(buf, gbuf, gbuf_len))
    return -EFAULT;

  *off = gbuf_len;
  return gbuf_len;
}

// write
static ssize_t drv_write(struct file *file, const char __user *buf, size_t size,
                         loff_t *off) {
  if (size > BUF_SIZE)
    size = BUF_SIZE;

  if (copy_from_user(gbuf, buf, size))
    return -EFAULT;

  gbuf_len = size;
  pr_info("записано %d байт\n", gbuf_len);
  return size;
}

// ioctl
static long drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
  if (cmd == IOCTL_CLEAR) {
    gbuf_len = 0;
    pr_info("буфер очищен\n");
    return 0;
  }

  if (cmd == IOCTL_HASDATA) {
    int val = (gbuf_len > 0);
    if (copy_to_user((int __user *)arg, &val, sizeof(int)))
      return -EFAULT;
    return 0;
  }

  return -EINVAL;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = drv_open,
    .release = drv_release,
    .read = drv_read,
    .write = drv_write,
    .unlocked_ioctl = drv_ioctl,
};

int init_module(void) {
  // выделяем номер устройства
  if (alloc_chrdev_region(&dev_num, 0, 1, "pz4") < 0)
    return -1;

  // создаём символьный драйвер
  cdev_init(&my_cdev, &fops);
  if (cdev_add(&my_cdev, dev_num, 1) < 0)
    return -1;

  // создаём класс
  my_class = class_create("pz4_class");
  if (IS_ERR(my_class))
    return -1;

  // создаём спецфайл автоматически
  device_create(my_class, NULL, dev_num, NULL, "pz4_dev");

  pr_info("драйвер загружен, major=%d\n", MAJOR(dev_num));
  return 0;
}

void cleanup_module(void) {
  device_destroy(my_class, dev_num);
  class_destroy(my_class);

  cdev_del(&my_cdev);
  unregister_chrdev_region(dev_num, 1);

  pr_info("драйвер выгружен\n");
}

MODULE_LICENSE("GPL");