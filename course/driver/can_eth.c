#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/net.h>
#include <net/sock.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/poll.h>
#include <linux/types.h>

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

static dev_t dev_num;
static struct cdev cdev;
static struct class *cls;
static struct device *dev;

static struct socket *udp_sock;
static struct task_struct *rx_thread;

static wait_queue_head_t rx_wq;
static DEFINE_MUTEX(lock);

static struct can_frame_simple rx_ring[RX_RING_SIZE];
static int rx_head;
static int rx_tail;
static int rx_count;

static struct can_eth_stats stats;

static int ring_push(struct can_frame_simple *f)
{
  if (rx_count >= RX_RING_SIZE)
    return -1;

  rx_ring[rx_head] = *f;
  rx_head = (rx_head + 1) % RX_RING_SIZE;
  rx_count++;
  return 0;
}

static int ring_pop(struct can_frame_simple *f)
{
  if (rx_count == 0)
    return -1;

  *f = rx_ring[rx_tail];
  rx_tail = (rx_tail + 1) % RX_RING_SIZE;
  rx_count--;
  return 0;
}

static int udp_send(struct can_frame_simple *f)
{
  struct sockaddr_in addr;
  struct msghdr msg = {};
  struct kvec iov;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(peer_port);

  if (!in4_pton(peer_ip, -1, (u8 *)&addr.sin_addr.s_addr, -1, NULL))
    return -EINVAL;

  msg.msg_name = &addr;
  msg.msg_namelen = sizeof(addr);

  iov.iov_base = f;
  iov.iov_len = sizeof(*f);

  return kernel_sendmsg(udp_sock, &msg, &iov, 1, iov.iov_len);
}

static int rx_thread_fn(void *arg)
{
  struct can_frame_simple f;
  struct msghdr msg = {};
  struct kvec iov;

  while (!kthread_should_stop()) {
    iov.iov_base = &f;
    iov.iov_len = sizeof(f);

    if (kernel_recvmsg(udp_sock, &msg, &iov, 1, sizeof(f),
                       MSG_DONTWAIT) <= 0) {
      msleep(10);
      continue;
    }

    if (f.dlc > 8)
      continue;

    mutex_lock(&lock);
    if (ring_push(&f) == 0) {
      stats.rx_frames++;
      wake_up_interruptible(&rx_wq);
    } else {
      stats.rx_dropped++;
    }
    mutex_unlock(&lock);
  }
  return 0;
}

static ssize_t dev_write(struct file *f, const char __user *buf,
                         size_t len, loff_t *off)
{
  struct can_frame_simple fr;

  if (len != sizeof(fr))
    return -EINVAL;

  if (copy_from_user(&fr, buf, sizeof(fr)))
    return -EFAULT;

  if (fr.dlc > 8)
    return -EINVAL;

  if (udp_send(&fr) < 0)
    return -EIO;

  mutex_lock(&lock);
  stats.tx_frames++;
  mutex_unlock(&lock);

  return sizeof(fr);
}

static ssize_t dev_read(struct file *f, char __user *buf,
                        size_t len, loff_t *off)
{
  struct can_frame_simple fr;

  if (len < sizeof(fr))
    return -EINVAL;

  wait_event_interruptible(rx_wq, rx_count > 0);

  mutex_lock(&lock);
  if (ring_pop(&fr) < 0) {
    mutex_unlock(&lock);
    return -EAGAIN;
  }
  mutex_unlock(&lock);

  if (copy_to_user(buf, &fr, sizeof(fr)))
    return -EFAULT;

  return sizeof(fr);
}

static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
  struct can_eth_stats st;

  switch (cmd) {
  case IOCTL_CLEAR_RX:
    mutex_lock(&lock);
    rx_head = rx_tail = rx_count = 0;
    mutex_unlock(&lock);
    return 0;

  case IOCTL_GET_STATS:
    mutex_lock(&lock);
    st = stats;
    mutex_unlock(&lock);

    if (copy_to_user((void __user *)arg, &st, sizeof(st)))
      return -EFAULT;
    return 0;
  }
  return -ENOTTY;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
};

static int __init mod_init(void)
{
  struct sockaddr_in addr;

  init_waitqueue_head(&rx_wq);

  alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
  cdev_init(&cdev, &fops);
  cdev_add(&cdev, dev_num, 1);

  cls = class_create(DEV_NAME);
  dev = device_create(cls, NULL, dev_num, NULL, DEV_NAME);

  sock_create_kern(&init_net, AF_INET, SOCK_DGRAM,
                   IPPROTO_UDP, &udp_sock);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(listen_port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  kernel_bind(udp_sock, (struct sockaddr *)&addr, sizeof(addr));

  rx_thread = kthread_run(rx_thread_fn, NULL, "can_eth_rx");
  return 0;
}

static void __exit mod_exit(void)
{
  if (rx_thread)
    kthread_stop(rx_thread);

  if (udp_sock)
    sock_release(udp_sock);

  device_destroy(cls, dev_num);
  class_destroy(cls);
  cdev_del(&cdev);
  unregister_chrdev_region(dev_num, 1);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");