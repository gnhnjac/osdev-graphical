#include <stdint.h>
#include "memory.h"
#include "network.h"
#include "process.h"

// general purpose functions

uint16_t switch_endian16(uint16_t nb) {
   return (nb>>8) | (nb<<8);
}

uint32_t switch_endian32(uint32_t nb) {
   return ((nb>>24)&0xff)      |
          ((nb<<8)&0xff0000)   |
          ((nb>>8)&0xff00)     |
          ((nb<<24)&0xff000000);
}

uint8_t get_byte_from32(uint32_t buf, int b)
{

	return ((buf&(0xFF<<(8*b)))>>(8*b));

}

// receive network stack

void ethernet_handle_packet(void *packet, int packet_length)
{

	PETHERNET_FRAME ethernet = (PETHERNET_FRAME)packet;

	uint8_t *dest_mac = (uint8_t *)&ethernet->dest_mac;
	uint8_t *src_mac = (uint8_t *)&ethernet->src_mac;
	uint16_t ethertype = switch_endian16(ethernet->ethertype);

	printf("\n--------------\nETHERTNET\n--------------\n");
	printf("SOURCE MAC: %X:%X:%X:%X:%X:%X\n",src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]);
	printf("DEST MAC: %X:%X:%X:%X:%X:%X\n",dest_mac[0],dest_mac[1],dest_mac[2],dest_mac[3],dest_mac[4],dest_mac[5]);
	printf("ETHERTYPE: %U\n",ethertype);

	switch(ethertype)
	{
		case ETHERTYPE_IPV4:
			ipv4_handle_packet(packet+sizeof(ETHERNET_FRAME),packet_length-sizeof(ETHERNET_FRAME));
			break;
		case ETHERTYPE_IPV6:
			ipv6_handle_packet(packet+sizeof(ETHERNET_FRAME),packet_length-sizeof(ETHERNET_FRAME));
			break;
		case ETHERTYPE_ARP:
			arp_handle_packet(packet+sizeof(ETHERNET_FRAME),packet_length-sizeof(ETHERNET_FRAME));
			break;
		default:
			printf("Link protocol not supported.\n");
	}
}

void arp_handle_packet(void *packet, int packet_length)
{

	printf("\n--------------\nARP\n--------------\n");

}

void ipv4_handle_packet(void *packet, int packet_length)
{

	PIPV4_FRAME ipv4 = (PIPV4_FRAME)packet;

	printf("\n--------------\nIPV4\n--------------\n");

	if (ipv4->version_ihl&IPV4_IHL_MASK > 5 || ((ipv4->version_ihl&IPV4_VERSION_MASK)>>4) != 4)
	{
		printf("IPv4 type not supported.");
		return;
	}

	uint8_t ttl = ipv4->ttl;
	uint32_t src_ip = ipv4->src_ip;
	uint32_t dst_ip = ipv4->dst_ip;

	printf("SOURCE IP: %d.%d.%d.%d\n",get_byte_from32(src_ip,0),get_byte_from32(src_ip,1),get_byte_from32(src_ip,2),get_byte_from32(src_ip,3));
	printf("DEST IP: %d.%d.%d.%d\n",get_byte_from32(dst_ip,0),get_byte_from32(dst_ip,1),get_byte_from32(dst_ip,2),get_byte_from32(dst_ip,3));
	printf("TTL: %d\n",ttl);

	transport_handle_packet(packet+sizeof(IPV4_FRAME),packet_length-sizeof(IPV4_FRAME),ipv4->protocol_num);

}

void ipv6_handle_packet(void *packet, int packet_length)
{

	PIPV6_FRAME ipv6 = (PIPV6_FRAME)packet;

	printf("\n--------------\nIPV6\n--------------\n");

	uint8_t ttl = ipv6->ttl;
	uint16_t *src_ip = ipv6->src_ip;
	uint16_t *dst_ip = ipv6->dst_ip;

	printf("SOURCE IP: %U:%U:%U:%U:%U:%U:%U:%U\n",src_ip[0],src_ip[1],src_ip[2],src_ip[3],src_ip[4],src_ip[5],src_ip[6],src_ip[7]);
	printf("DEST IP: %U:%U:%U:%U:%U:%U:%U:%U\n",dst_ip[0],dst_ip[1],dst_ip[2],dst_ip[3],dst_ip[4],dst_ip[5],dst_ip[6],dst_ip[7]);
	printf("TTL: %d\n",ttl);

	transport_handle_packet(packet+sizeof(IPV6_FRAME),packet_length-sizeof(IPV6_FRAME),ipv6->protocol_num);

}

void transport_handle_packet(void *packet, int packet_length, uint8_t prot_num)
{

	switch(prot_num)
	{

		case IP_PROTOCOL_ICMP:
			printf("ICMP protocol not supported.\n");
			break;
		case IP_PROTOCOL_TCP:
			printf("TCP protocol not supported.\n");
			break;
		case IP_PROTOCOL_UDP:
			printf("UDP protocol not supported.\n");
			break;
		default:
			printf("Transport protocol %d not supported.\n",prot_num);

	}

}