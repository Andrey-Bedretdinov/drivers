#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/pci.h>

#define MY_VENDOR 0x8086
#define MY_DEVICE 0x10d3

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

  pr_info("pci_net: probe вызван для устройства %04x:%04x\n", id->vendor,
          id->device);

  pci_set_master(pdev);

  if (pci_enable_device(pdev))
    return -ENODEV;

  // создаем сетевое устройство
  ndev = alloc_etherdev(0);
  if (!ndev)
    return -ENOMEM;

  // привязываем netdev к pci_dev
  pci_set_drvdata(pdev, ndev);

  void __iomem *mmio;
  u32 ral, rah;

  mmio = pci_iomap(pdev, 0, 0);
  if (!mmio) {
    pr_err("pci_net: не удалось ioremap BAR0\n");
    free_netdev(ndev);
    return -ENODEV;
  }

  ndev->mem_start = (unsigned long)mmio;

  ral = readl(mmio + 0x5400);
  rah = readl(mmio + 0x5404);

  unsigned char mac[ETH_ALEN];

  mac[0] = ral & 0xff;
  mac[1] = (ral >> 8) & 0xff;
  mac[2] = (ral >> 16) & 0xff;
  mac[3] = (ral >> 24) & 0xff;
  mac[4] = rah & 0xff;
  mac[5] = (rah >> 8) & 0xff;

  eth_hw_addr_set(ndev, mac);

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

  void __iomem *mmio = (void __iomem *) dev->mem_start;

  unregister_netdev(dev);
  pci_iounmap(pdev, mmio);
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