#ifndef CAN_ETH_UAPI_H
#define CAN_ETH_UAPI_H

#include <stdint.h>
#include <sys/ioctl.h>

#define CAN_ETH_MAGIC 'q'

struct can_frame_simple {
  uint32_t id;
  uint8_t dlc;
  uint8_t data[8];
};

struct can_eth_stats {
  uint64_t tx_frames;
  uint64_t rx_frames;
  uint64_t rx_dropped;
};

#define IOCTL_CLEAR_RX   _IO(CAN_ETH_MAGIC, 1)
#define IOCTL_GET_STATS  _IOR(CAN_ETH_MAGIC, 2, struct can_eth_stats)

#endif