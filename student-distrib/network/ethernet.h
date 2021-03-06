#ifndef _ETHERNET_H
#define _ETHERNET_H

#include "../lib.h"
#include "network_misc.h"

// Uncomment ETH_DEBUG_ENABLE to enable debugging
// #define ETH_DEBUG_ENABLE
#ifdef ETH_DEBUG_ENABLE
	#define ETH_DEBUG(f, ...) printf(f, ##__VA_ARGS__)
#else
	#define ETH_DEBUG(f, ...) // Nothing
#endif

// The offsets of a MAC address
#define DST_MAC_ADDR_OFFSET 0
#define SRC_MAC_ADDR_OFFSET 6
// The size and offset of the EtherType field
#define ETHER_TYPE_SIZE 2
#define ETHER_TYPE_OFFSET 12
// The offset of the EtherType field if this is a VLAN tagged packet (802.1Q)
#define VLAN_ETHER_TYPE_OFFSET 16
// The offset of PCP/DEI/VID (packed in big endian order with bit sizes 3, 1, 12)
//  if it's a VLAN tagged packet
#define PCP_DEI_VID_OFFSET 14
// The offset of the payload if and if not it is a VLAN tagged packet
#define PAYLOAD_OFFSET 14
#define VLAN_PAYLOAD_OFFSET 18  
// The size of the CRC checksum
#define CRC_SIZE 4

// Various EtherType values
// EtherType for VLAN
#define ET_VLAN 0x8100
// EtherType for IPv4
#define ET_IPV4 0x0800
// EtherType for Address Resolution Protocol (ARP)
#define ET_ARP 0x0806

// Receives an Ethernet packet, performs appropriate actions, and replies if necessary
// To be called by interrupt handlers
int receive_eth_packet(uint8_t *buffer, uint32_t length, uint32_t id);
// Generates an Ethernet packet with the specified fields and transmits it
int send_eth_packet(uint8_t dest_mac_addr[MAC_ADDR_SIZE], uint16_t ether_type, void *payload, uint32_t payload_size, uint32_t id);

#endif
