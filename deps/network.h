#define ETHER_SIZE 6+6+2 // dest mac + src mac + ethertype field


//refs
void build_ethernet_layer(char *packet, char *dest_mac, char* src_mac, uint16_t ether_type);