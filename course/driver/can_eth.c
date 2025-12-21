#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/types.h>

#include <linux/net.h>
#include <net/sock.h>
#include <linux/in.h>
#include <linux/inet.h>

#define DEV_NAME "can_eth"
#define RX_RING_SIZE 128

#define CAN_ETH_MAGIC 'q'
#define IOCTL_CLEAR_RX  _IO(CAN_ETH_MAGIC, 1)
#define IOCTL_GET_STATS _IOR(CAN_ETH_MAGIC, 2, struct can_eth_stats)

struct can_frame_simple {
  u32 id;
  u8  dlc;
  u8  data[8];
};

struct can_eth_stats {
  u64 tx_frames;
  u64 rx_frames;
  u64 rx_dropped;
};

static char *peer_ip = "127.0.0.1";
static int peer_port = 15000;
static int listen_port = 15000;

module_param(peer_ip, charp, 0644);
module_param(peer_port, int, 0644);
module_param(listen_port, int, 0644);

static dev_t g_devnum;
static struct cdev g_cdev;
static struct class *g_class;
static struct device *g_device;

static struct socket *g_udp_sock;
static struct task_struct *g_rx_thread;

static wait_queue_head_t g_rx_wq;
static DEFINE_MUTEX(g_lock);

static struct can_frame_simple g_rx_ring[RX_RING_SIZE];
static int g_rx_head;
static int g_rx_tail;
static int g_rx_count;

static struct can_eth_stats g_stats;

static int ring_push(const struct can_frame_simple *f)
{
  if (g_rx_count >= RX_RING_SIZE)
    return -1;

  g_rx_ring[g_rx_head] = *f;
  g_rx_head = (g_rx_head + 1) % RX_RING_SIZE;
  g_rx_count++;
  return 0;
}

static int ring_pop(struct can_frame_simple *f)
{
  if (g_rx_count == 0)
    return -1;

  *f = g_rx_ring[g_rx_tail];
  g_rx_tail = (g_rx_tail + 1) % RX_RING_SIZE;
  g_rx_count--;
  return 0;
}

static int udp_bind(int port)
{
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((u16)port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  return kernel_bind(g_udp_sock, (struct sockaddr *)&addr, sizeof(addr));
}

static int udp_send_frame(const struct can_frame_simple *f)
{
  struct sockaddr_in addr;
  struct msghdr msg;
  struct kvec iov;
  int ret;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((u16)peer_port);

  if (!in4_pton(peer_ip, -1, (u8 *)&addr.sin_addr.s_addr, -1, NULL))
    return -EINVAL;

  memset(&msg, 0, sizeof(msg));
  msg.msg_name = &addr;
  msg.msg_namelen = sizeof(addr);

  iov.iov_base = (void *)f;
  iov.iov_len = sizeof(*f);

  ret = kernel_sendmsg(g_udp_sock, &msg, &iov, 1, iov.iov_len);
  if (ret < 0)
    return ret;

  if (ret != sizeof(*f))
    return -EIO;

  return 0;
}

static int rx_thread_fn(void *arg)
{
  struct can_frame_simple f;
  struct msghdr msg;
  struct kvec iov;
  int ret;

  while (!kthread_should_stop()) {
    memset(&msg, 0, sizeof(msg));
    memset(&f, 0, sizeof(f));

    iov.iov_base = &f;
    iov.iov_len = sizeof(f);

    ret = kernel_recvmsg(g_udp_sock, &msg, &iov, 1, sizeof(f), MSG_DONTWAIT);
    if (ret == -EAGAIN || ret == -EWOULDBLOCK) {
      msleep(10);
      continue;
    }

    if (ret == -EINTR)
      continue;

    if (ret < 0) {
      msleep(10);
      continue;
    }

    if (ret != sizeof(f)) {
      mutex_lock(&g_lock);
      g_stats.rx_dropped++;
      mutex_unlock(&g_lock);
      continue;
    }

    if (f.dlc > 8) {
      mutex_lock(&g_lock);
      g_stats.rx_dropped++;
      mutex_unlock(&g_lock);
      continue;
    }

    mutex_lock(&g_lock);
    if (ring_push(&f) == 0) {
      g_stats.rx_frames++;
      wake_up_interruptible(&g_rx_wq);
    } else {
      g_stats.rx_dropped++;
    }
    mutex_unlock(&g_lock);
  }

  wake_up_interruptible(&g_rx_wq);
  return 0;
}

static ssize_t caneth_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
  struct can_frame_simple f;
  int ret;

  if (len != sizeof(f))
    return -EINVAL;

  if (copy_from_user(&f, buf, sizeof(f)))
    return -EFAULT;

  if (f.dlc > 8)
    return -EINVAL;

  ret = udp_send_frame(&f);
  if (ret < 0)
    return ret;

  mutex_lock(&g_lock);
  g_stats.tx_frames++;
  mutex_unlock(&g_lock);

  return sizeof(f);
}

static ssize_t caneth_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
  struct can_frame_simple f;
  int ret;

  if (len < sizeof(f))
    return -EINVAL;

  if (filp->f_flags & O_NONBLOCK) {
    mutex_lock(&g_lock);
    ret = ring_pop(&f);
    mutex_unlock(&g_lock);

    if (ret < 0)
      return -EAGAIN;
  } else {
    ret = wait_event_interruptible(g_rx_wq, g_rx_count > 0 || kthread_should_stop());
    if (ret)
      return ret;

    mutex_lock(&g_lock);
    ret = ring_pop(&f);
    mutex_unlock(&g_lock);

    if (ret < 0)
      return -EAGAIN;
  }

  if (copy_to_user(buf, &f, sizeof(f)))
    return -EFAULT;

  return sizeof(f);
}

static __poll_t caneth_poll(struct file *filp, poll_table *wait)
{
  __poll_t mask = 0;

  poll_wait(filp, &g_rx_wq, wait);

  mutex_lock(&g_lock);
  if (g_rx_count > 0)
    mask |= POLLIN | POLLRDNORM;
  mutex_unlock(&g_lock);

  return mask;
}

static long caneth_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  struct can_eth_stats st;

  switch (cmd) {
  case IOCTL_CLEAR_RX:
    mutex_lock(&g_lock);
    g_rx_head = 0;
    g_rx_tail = 0;
    g_rx_count = 0;
    mutex_unlock(&g_lock);
    return 0;

  case IOCTL_GET_STATS:
    mutex_lock(&g_lock);
    st = g_stats;
    mutex_unlock(&g_lock);

    if (copy_to_user((void __user *)arg, &st, sizeof(st)))
      return -EFAULT;
    return 0;

  default:
    return -ENOTTY;
  }
}

static const struct file_operations g_fops = {
    .owner = THIS_MODULE,
    .read = caneth_read,
    .write = caneth_write,
    .unlocked_ioctl = caneth_ioctl,
    .poll = caneth_poll,
};

static int __init caneth_init(void)
{
  int ret;

  init_waitqueue_head(&g_rx_wq);

  ret = alloc_chrdev_region(&g_devnum, 0, 1, DEV_NAME);
  if (ret < 0)
    return ret;

  cdev_init(&g_cdev, &g_fops);
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

  ret = udp_bind(listen_port);
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

static void __exit caneth_exit(void)
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

module_init(caneth_init);
module_exit(caneth_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("andrey");
MODULE_DESCRIPTION("simple can frames over udp through char device");