#include <linux/cdev.h>
#include <linux/class.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/net.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#include <net/sock.h>
#include <linux/in.h>
#include <linux/inet.h>

#include "can_eth_uapi.h"

#define DEV_NAME "can_eth"
#define RX_RING_SIZE 1024

static char *peer_ip = "127.0.0.1";
static int peer_port = 15000;
static int listen_port = 15000;

module_param(peer_ip, charp, 0644);
module_param(peer_port, int, 0644);
module_param(listen_port, int, 0644);

MODULE_PARM_DESC(peer_ip, "udp peer ip like 192.168.1.10");
MODULE_PARM_DESC(peer_port, "udp peer port");
MODULE_PARM_DESC(listen_port, "udp listen port");

static dev_t g_devnum;
static struct cdev g_cdev;
static struct class *g_class;
static struct device *g_device;

static struct socket *g_udp_sock;
static struct task_struct *g_rx_thread;

static wait_queue_head_t g_rx_wq;
static struct mutex g_rx_lock;

static struct can_frame_simple g_rx_ring[RX_RING_SIZE];
static unsigned int g_rx_head;
static unsigned int g_rx_tail;
static unsigned int g_rx_count;

static struct can_eth_stats g_stats;

static int ring_push(const struct can_frame_simple *fr)
{
  if (g_rx_count >= RX_RING_SIZE)
    return -ENOSPC;

  g_rx_ring[g_rx_head] = *fr;
  g_rx_head = (g_rx_head + 1) % RX_RING_SIZE;
  g_rx_count++;
  return 0;
}

static int ring_pop(struct can_frame_simple *fr)
{
  if (g_rx_count == 0)
    return -ENOENT;

  *fr = g_rx_ring[g_rx_tail];
  g_rx_tail = (g_rx_tail + 1) % RX_RING_SIZE;
  g_rx_count--;
  return 0;
}

static int udp_send_frame(const struct can_frame_simple *fr)
{
  struct sockaddr_in dst;
  struct msghdr msg;
  struct kvec iov;
  int ret;

  memset(&dst, 0, sizeof(dst));
  dst.sin_family = AF_INET;
  dst.sin_port = htons((uint16_t)peer_port);

  ret = in4_pton(peer_ip, -1, (u8 *)&dst.sin_addr.s_addr, -1, NULL);
  if (ret == 0)
    return -EINVAL;

  memset(&msg, 0, sizeof(msg));
  msg.msg_name = &dst;
  msg.msg_namelen = sizeof(dst);

  iov.iov_base = (void *)fr;
  iov.iov_len = sizeof(*fr);

  ret = kernel_sendmsg(g_udp_sock, &msg, &iov, 1, iov.iov_len);
  if (ret < 0)
    return ret;

  if (ret != sizeof(*fr))
    return -EIO;

  return 0;
}

static int udp_bind_socket(int port)
{
  struct sockaddr_in addr;
  int ret;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((uint16_t)port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  ret = kernel_bind(g_udp_sock, (struct sockaddr *)&addr, sizeof(addr));
  return ret;
}

static int rx_thread_fn(void *arg)
{
  struct msghdr msg;
  struct kvec iov;
  struct can_frame_simple fr;
  int ret;

  while (!kthread_should_stop()) {
    memset(&msg, 0, sizeof(msg));
    iov.iov_base = &fr;
    iov.iov_len = sizeof(fr);

    ret = kernel_recvmsg(g_udp_sock, &msg, &iov, 1, sizeof(fr), 0);
    if (ret == -EINTR)
      continue;

    if (ret < 0) {
      if (kthread_should_stop())
        break;
      continue;
    }

    if (ret != sizeof(fr)) {
      mutex_lock(&g_rx_lock);
      g_stats.rx_dropped++;
      mutex_unlock(&g_rx_lock);
      continue;
    }

    if (fr.dlc > 8) {
      mutex_lock(&g_rx_lock);
      g_stats.rx_dropped++;
      mutex_unlock(&g_rx_lock);
      continue;
    }

    mutex_lock(&g_rx_lock);
    if (ring_push(&fr) == 0) {
      g_stats.rx_frames++;
      wake_up_interruptible(&g_rx_wq);
    } else {
      g_stats.rx_dropped++;
    }
    mutex_unlock(&g_rx_lock);
  }

  return 0;
}

static int dev_open(struct inode *inode, struct file *file)
{
  return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
  return 0;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
  struct can_frame_simple fr;
  int ret;

  if (len != sizeof(fr))
    return -EINVAL;

  if (copy_from_user(&fr, buf, sizeof(fr)))
    return -EFAULT;

  if (fr.dlc > 8)
    return -EINVAL;

  ret = udp_send_frame(&fr);
  if (ret < 0)
    return ret;

  mutex_lock(&g_rx_lock);
  g_stats.tx_frames++;
  mutex_unlock(&g_rx_lock);

  return sizeof(fr);
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
  struct can_frame_simple fr;
  int ret;

  if (len < sizeof(fr))
    return -EINVAL;

  if (file->f_flags & O_NONBLOCK) {
    mutex_lock(&g_rx_lock);
    ret = ring_pop(&fr);
    mutex_unlock(&g_rx_lock);
    if (ret < 0)
      return -EAGAIN;
  } else {
    ret = wait_event_interruptible(g_rx_wq, g_rx_count > 0);
    if (ret)
      return ret;

    mutex_lock(&g_rx_lock);
    ret = ring_pop(&fr);
    mutex_unlock(&g_rx_lock);
    if (ret < 0)
      return -EAGAIN;
  }

  if (copy_to_user(buf, &fr, sizeof(fr)))
    return -EFAULT;

  return sizeof(fr);
}

static __poll_t dev_poll(struct file *file, poll_table *wait)
{
  __poll_t mask = 0;

  poll_wait(file, &g_rx_wq, wait);

  mutex_lock(&g_rx_lock);
  if (g_rx_count > 0)
    mask |= POLLIN | POLLRDNORM;
  mutex_unlock(&g_rx_lock);

  return mask;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  struct can_eth_stats st;

  switch (cmd) {
  case IOCTL_CLEAR_RX:
    mutex_lock(&g_rx_lock);
    g_rx_head = 0;
    g_rx_tail = 0;
    g_rx_count = 0;
    mutex_unlock(&g_rx_lock);
    return 0;

  case IOCTL_GET_STATS:
    mutex_lock(&g_rx_lock);
    st = g_stats;
    mutex_unlock(&g_rx_lock);

    if (copy_to_user((void __user *)arg, &st, sizeof(st)))
      return -EFAULT;
    return 0;

  default:
    return -ENOTTY;
  }
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
    .poll = dev_poll,
};

static int __init can_eth_init(void)
{
  int ret;

  init_waitqueue_head(&g_rx_wq);
  mutex_init(&g_rx_lock);

  ret = alloc_chrdev_region(&g_devnum, 0, 1, DEV_NAME);
  if (ret < 0)
    return ret;

  cdev_init(&g_cdev, &fops);
  g_cdev.owner = THIS_MODULE;

  ret = cdev_add(&g_cdev, g_devnum, 1);
  if (ret < 0)
    goto err_chrdev;

  g_class = class_create(DEV_NAME);
  if (IS_ERR(g_class)) {
    ret = PTR_ERR(g_class);
    g_class = NULL;
    goto err_cdev;
  }

  g_device = device_create(g_class, NULL, g_devnum, NULL, DEV_NAME);
  if (IS_ERR(g_device)) {
    ret = PTR_ERR(g_device);
    g_device = NULL;
    goto err_class;
  }

  ret = sock_create_kern(&init_net, AF_INET, SOCK_DGRAM, IPPROTO_UDP, &g_udp_sock);
  if (ret < 0)
    goto err_devnode;

  ret = udp_bind_socket(listen_port);
  if (ret < 0)
    goto err_sock;

  g_rx_thread = kthread_run(rx_thread_fn, NULL, "can_eth_rx");
  if (IS_ERR(g_rx_thread)) {
    ret = PTR_ERR(g_rx_thread);
    g_rx_thread = NULL;
    goto err_sock;
  }

  return 0;

err_sock:
  if (g_udp_sock) {
    sock_release(g_udp_sock);
    g_udp_sock = NULL;
  }
err_devnode:
  if (g_device) {
    device_destroy(g_class, g_devnum);
    g_device = NULL;
  }
err_class:
  if (g_class) {
    class_destroy(g_class);
    g_class = NULL;
  }
err_cdev:
  cdev_del(&g_cdev);
err_chrdev:
  unregister_chrdev_region(g_devnum, 1);
  return ret;
}

static void __exit can_eth_exit(void)
{
  if (g_rx_thread) {
    kthread_stop(g_rx_thread);
    g_rx_thread = NULL;
  }

  if (g_udp_sock) {
    sock_release(g_udp_sock);
    g_udp_sock = NULL;
  }

  if (g_device) {
    device_destroy(g_class, g_devnum);
    g_device = NULL;
  }

  if (g_class) {
    class_destroy(g_class);
    g_class = NULL;
  }

  cdev_del(&g_cdev);
  unregister_chrdev_region(g_devnum, 1);
}

module_init(can_eth_init);
module_exit(can_eth_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("andrey");
MODULE_DESCRIPTION("simple can frames over udp through char device");