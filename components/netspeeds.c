/* See LICENSE file for copyright and license details. */
#include <limits.h>
#include <stdio.h>

#include "../slstatus.h"
#include "../util.h"

#if defined(__linux__)
#include <stdint.h>

#define NET_RX_BYTES "/sys/class/net/%s/statistics/rx_bytes"
#define NET_TX_BYTES "/sys/class/net/%s/statistics/tx_bytes"

const char *netspeed_rx_all() {
  const char *ifaces[] = {"wlan0", "eth0"};
  static uintmax_t rx_bytes_all = 0;
  uintmax_t tmp = 0, amt = 0, oldrxbytes;
  extern const unsigned int interval;
  char path[PATH_MAX];

  oldrxbytes = rx_bytes_all;

  for (int i = 0; i < 2; ++i) {
    if (esnprintf(path, sizeof(path), NET_RX_BYTES, ifaces[i]) < 0)
      tmp += 0;
    if (pscanf(path, "%ju", &tmp) != 1)
      tmp += 0;
    amt += tmp;
  }

  rx_bytes_all = amt;

  if (oldrxbytes == 0)
    return NULL;

  return fmt_human((rx_bytes_all - oldrxbytes) * 1000 / interval, 1024);
}

const char *netspeed_tx_all() {
  const char *ifaces[] = {"wlan0", "eth0"};
  static uintmax_t tx_bytes_all;
  uintmax_t tmp = 0, amt = 0, oldtxbytes;
  extern const unsigned int interval;
  char path[PATH_MAX];

  oldtxbytes = tx_bytes_all;

  for (int i = 0; i < 2; ++i) {
    if (esnprintf(path, sizeof(path), NET_TX_BYTES, ifaces[i]) < 0)
      tmp += 0;
    if (pscanf(path, "%ju", &tmp) != 1)
      tmp += 0;
    amt += tmp;
  }

  tx_bytes_all = amt;

  if (oldtxbytes == 0)
    return NULL;

  return fmt_human((tx_bytes_all - oldtxbytes) * 1000 / interval, 1024);
}

const char *netspeed_rx(const char *interface) {
  uintmax_t oldrxbytes;
  static uintmax_t rxbytes;
  extern const unsigned int interval;
  char path[PATH_MAX];

  oldrxbytes = rxbytes;

  if (esnprintf(path, sizeof(path), NET_RX_BYTES, interface) < 0)
    return NULL;
  if (pscanf(path, "%ju", &rxbytes) != 1)
    return NULL;
  if (oldrxbytes == 0)
    return NULL;

  return fmt_human((rxbytes - oldrxbytes) * 1000 / interval, 1024);
}

const char *netspeed_tx(const char *interface) {
  uintmax_t oldtxbytes;
  static uintmax_t txbytes;
  extern const unsigned int interval;
  char path[PATH_MAX];

  oldtxbytes = txbytes;

  if (esnprintf(path, sizeof(path), NET_TX_BYTES, interface) < 0)
    return NULL;
  if (pscanf(path, "%ju", &txbytes) != 1)
    return NULL;
  if (oldtxbytes == 0)
    return NULL;

  return fmt_human((txbytes - oldtxbytes) * 1000 / interval, 1024);
}

#elif defined(__OpenBSD__) | defined(__FreeBSD__)
#include <ifaddrs.h>
#include <net/if.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

const char *netspeed_rx(const char *interface) {
  struct ifaddrs *ifal, *ifa;
  struct if_data *ifd;
  uintmax_t oldrxbytes;
  static uintmax_t rxbytes;
  extern const unsigned int interval;
  int if_ok = 0;

  oldrxbytes = rxbytes;

  if (getifaddrs(&ifal) < 0) {
    warn("getifaddrs failed");
    return NULL;
  }
  rxbytes = 0;
  for (ifa = ifal; ifa; ifa = ifa->ifa_next)
    if (!strcmp(ifa->ifa_name, interface) &&
        (ifd = (struct if_data *)ifa->ifa_data))
      rxbytes += ifd->ifi_ibytes, if_ok = 1;

  freeifaddrs(ifal);
  if (!if_ok) {
    warn("reading 'if_data' failed");
    return NULL;
  }
  if (oldrxbytes == 0)
    return NULL;

  return fmt_human((rxbytes - oldrxbytes) * 1000 / interval, 1024);
}

const char *netspeed_tx(const char *interface) {
  struct ifaddrs *ifal, *ifa;
  struct if_data *ifd;
  uintmax_t oldtxbytes;
  static uintmax_t txbytes;
  extern const unsigned int interval;
  int if_ok = 0;

  oldtxbytes = txbytes;

  if (getifaddrs(&ifal) < 0) {
    warn("getifaddrs failed");
    return NULL;
  }
  txbytes = 0;
  for (ifa = ifal; ifa; ifa = ifa->ifa_next)
    if (!strcmp(ifa->ifa_name, interface) &&
        (ifd = (struct if_data *)ifa->ifa_data))
      txbytes += ifd->ifi_obytes, if_ok = 1;

  freeifaddrs(ifal);
  if (!if_ok) {
    warn("reading 'if_data' failed");
    return NULL;
  }
  if (oldtxbytes == 0)
    return NULL;

  return fmt_human((txbytes - oldtxbytes) * 1000 / interval, 1024);
}
#endif
