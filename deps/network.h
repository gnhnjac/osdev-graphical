#include <stdint.h>

#pragma pack(1)


typedef struct _ETHERNET_FRAME
{

	uint8_t dest_mac[6];
	uint8_t src_mac[6];
	uint16_t ethertype;

} ETHERNET_FRAME, *PETHERNET_FRAME;

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_IPV6 0x86DD
#define ETHERTYPE_ARP 0x0806

typedef struct _IPV4_FRAME
{

	uint8_t version_ihl; // version=4, length of the header in 32 bit chunks
	uint8_t dscp_ecn;
	uint16_t total_length; // packet size
	uint16_t identification;
	uint16_t flags_fragoff;
	uint8_t ttl;
	uint8_t protocol_num;
	uint16_t header_checksum;
	uint32_t src_ip;
	uint32_t dst_ip;

} IPV4_FRAME, *PIPV4_FRAME;

#define IPV4_VERSION_MASK 0xF0
#define IPV4_IHL_MASK 0xF

typedef struct _IPV6_FRAME
{

	uint8_t version_traffic_class_flow_label; // version=6
	uint16_t payload_length_in_bytes;
	uint8_t protocol_num;
	uint8_t ttl;
	uint16_t src_ip[8];
	uint16_t dst_ip[8];

} IPV6_FRAME, *PIPV6_FRAME;

typedef enum _ip_protocol
{
	IP_PROTOCOL_ICMP = 0x1,
	IP_PROTOCOL_TCP = 0x6,
	IP_PROTOCOL_UDP = 0x11
} ip_protocol;

#pragma pack()

//refs
uint16_t switch_endian16(uint16_t nb);
uint32_t switch_endian32(uint32_t nb);
uint8_t get_byte_from32(uint32_t buf, int b);
void ethernet_handle_packet(void *packet, int packet_length);
void arp_handle_packet(void *packet, int packet_length);
void ipv4_handle_packet(void *packet, int packet_length);
void ipv6_handle_packet(void *packet, int packet_length);
void transport_handle_packet(void *packet, int packet_length, uint8_t prot_num);