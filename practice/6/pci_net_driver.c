#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/pci.h>

#define MY_VENDOR 0x1af4
#define MY_DEVICE 0x1000

static struct net_device *ndev;

// open
static int net_open(struct net_device *dev) {
  pr_info("pci_net: интерфейс открыт\n");
  netif_start_queue(dev);
  return 0;
}

// release
static int net_stop(struct net_device *dev) {
  pr_info("pci_net: интерфейс остановлен\n");
  netif_stop_queue(dev);
  return 0;
}

// имитация отправки пакета
static netdev_tx_t net_xmit(struct sk_buff *skb, struct net_device *dev) {
  pr_info("pci_net: пакет получен в hard_start_xmit, длина=%d байт\n",
          skb->len);

  // вывод первых байт пакета
  print_hex_dump(KERN_INFO, "pci_net пакет: ", DUMP_PREFIX_OFFSET, 16, 1,
                 skb->data, skb->len, true);

  dev->stats.tx_packets++;
  dev->stats.tx_bytes += skb->len;

  dev_kfree_skb(skb);
  return NETDEV_TX_OK;
}

static const struct net_device_ops net_ops = {
    .ndo_open = net_open,
    .ndo_stop = net_stop,
    .ndo_start_xmit = net_xmit,
};

// probe PCI
static int pci_demo_probe(struct pci_dev *pdev,
                          const struct pci_device_id *id) {
  int i;

  pr_info("pci_net: probe вызван для устройства %04x:%04x\n", id->vendor,
          id->device);

  if (pci_enable_device(pdev))
    return -ENODEV;

  // создаем сетевое устройство
  ndev = alloc_etherdev(0);
  if (!ndev)
    return -ENOMEM;

  // привязываем netdev к pci_dev
  pci_set_drvdata(pdev, ndev);

  // читаем MAC из PCI config space (6 байт)
  for (i = 0; i < ETH_ALEN; i++)
    pci_read_config_byte(pdev, 0x10 + i, &ndev->dev_addr[i]);

  pr_info("pci_net: MAC-адрес = %pM\n", ndev->dev_addr);

  ndev->netdev_ops = &net_ops;

  // регистрируем интерфейс
  if (register_netdev(ndev)) {
    pr_info("pci_net: ошибка при регистрации интерфейса\n");
    free_netdev(ndev);
    return -ENODEV;
  }

  pr_info("pci_net: интерфейс зарегистрирован как %s\n", ndev->name);

  return 0;
}

// remove PCI
static void pci_demo_remove(struct pci_dev *pdev) {
  struct net_device *dev = pci_get_drvdata(pdev);

  pr_info("pci_net: remove вызван, модуль выгружается\n");

  unregister_netdev(dev);
  free_netdev(dev);

  pci_disable_device(pdev);

  pr_info("pci_net: модуль успешно выгружен\n");
}

// таблица устройств
static const struct pci_device_id pci_demo_ids[] = {
    {PCI_DEVICE(MY_VENDOR, MY_DEVICE)}, {0}};
MODULE_DEVICE_TABLE(pci, pci_demo_ids);

static struct pci_driver pci_demo_driver = {
    .name = "pci_net_demo",
    .id_table = pci_demo_ids,
    .probe = pci_demo_probe,
    .remove = pci_demo_remove,
};

module_pci_driver(pci_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("andrey");
MODULE_DESCRIPTION("PCI сетевой драйвер");