#include <stdint.h>
#include "memory.h"

void build_ethernet_layer(char *packet, char *dest_mac, char* src_mac, uint16_t ether_type)
{

	memcpy(packet,dest_mac,6);
	memcpy(packet+6,src_mac,6);
	memcpy(packet+12,&ether_type,2);
}