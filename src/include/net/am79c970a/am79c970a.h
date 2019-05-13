#ifndef _NET_AM79C970A_H
#define _NET_AM79C970A_H

#include <sys/config.h>

#ifdef _CONFIG_NET_AM79C970A_

#include <stdint.h>
#include <sys/pci.h>

void am79c970a_init();
int32 am79c970a_get_info_from_pci();
int32 am79c970a_config(uint32 ioaddr, int shared);

int am79c970a_transmit(uint8 *data_buf, uint32 data_len);
void am79c970a_handler(int32 irq);

#endif

#endif 