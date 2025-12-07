#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>

struct net_device *netdemo_dev;

// вызывается при открытии интерфейса (ifconfig netdemo0 up)
static int netdemo_open(struct net_device *dev) {
  pr_info("netdemo: interface opened\n");
  netif_start_queue(dev);
  return 0;
}

// вызывается при закрытии интерфейса
static int netdemo_stop(struct net_device *dev) {
  pr_info("netdemo: interface stopped\n");
  netif_stop_queue(dev);
  return 0;
}

// печатаем пакет, пришедший в ndo_start_xmit
static int netdemo_xmit(struct sk_buff *skb, struct net_device *dev) {
  pr_info("netdemo: packet received in hard_start_xmit, len=%u\n", skb->len);

  // печатаем первые байты пакета
  print_hex_dump(KERN_INFO, "netdemo packet: ", DUMP_PREFIX_NONE, 16, 1,
                 skb->data, skb->len, true);

  dev->stats.tx_packets++;
  dev->stats.tx_bytes += skb->len;

  // освобождаем пакет
  dev_kfree_skb(skb);

  return NETDEV_TX_OK;
}

// простая инициализация
static int netdemo_init(struct net_device *dev) {
  pr_info("netdemo: initialized\n");
  return 0;
}

static const struct net_device_ops netdemo_ops = {
    .ndo_init = netdemo_init,
    .ndo_open = netdemo_open,
    .ndo_stop = netdemo_stop,
    .ndo_start_xmit = netdemo_xmit,
};

// первоначальная настройка интерфейса
static void netdemo_setup(struct net_device *dev) {
  dev->netdev_ops = &netdemo_ops;

  // задаём тип Ethernet-устройства
  ether_setup(dev);

  // чтобы он имел корректный MAC
  eth_hw_addr_random(dev);

  pr_info("netdemo: assigned random mac %pM\n", dev->dev_addr);
}

// загрузка модуля
int init_module(void) {
  int res;

  // создаём сетевое устройство
  netdemo_dev = alloc_netdev(0, "netdemo%d", NET_NAME_UNKNOWN, netdemo_setup);
  if (!netdemo_dev) {
    pr_info("netdemo: alloc_netdev failed\n");
    return -ENOMEM;
  }

  // регистрируем устройство
  res = register_netdev(netdemo_dev);
  if (res) {
    pr_info("netdemo: register_netdev failed %d\n", res);
    free_netdev(netdemo_dev);
    return res;
  }

  pr_info("netdemo: module loaded\n");
  return 0;
}

// выгрузка
void cleanup_module(void) {
  unregister_netdev(netdemo_dev);
  free_netdev(netdemo_dev);
  pr_info("netdemo: module unloaded\n");
}

MODULE_LICENSE("GPL");