#ifndef _NET_ETHERNET_ARP_H
#define _NET_ETHERNET_ARP_H

#include <sys/config.h>

#ifdef _CONFIG_NET_ETHERNET_

#include <stdint.h>
#include <types.h>
#include <net/ethernet/ethernet.h>

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY   2
 
#pragma pack(push, 1)
struct arp_hdr {
    uint16  hardware_type;
    uint16  protocol_type;
    uint8   hardware_len;
    uint8   protocol_len;
    uint16  opcode;
    uint8   source_hw_addr[ETH_ADDR_LEN];
    uint32  source_protocol_addr;
    uint8   target_hw_addr[ETH_ADDR_LEN];
    uint32  target_protocol_addr;
 
};
#pragma pack(pop)

void arp_hdr_init(struct arp_hdr *arp_hdr, uint16 hw_type, uint16 proto_type, uint8 hw_len, uint8 proto_len, \
            uint16 opcode, uint8 src_hw_addr[ETH_ADDR_LEN], uint32 src_proto_addr, \
            uint8 tgt_hw_addr[ETH_ADDR_LEN], uint32 tgt_proto_addr);
			
void arp_request(uint32 ip);

#endif

#endif 