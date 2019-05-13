#ifndef _NET_AM79C973_H
#define _NET_AM79C973_H

#include <sys/config.h>

#ifdef _CONFIG_NET_AM79C973_

#include <stdint.h>
#include <sys/pci.h>

void am79c973_init();
void am79c973_handler(int32 irq);

#endif

#endif 