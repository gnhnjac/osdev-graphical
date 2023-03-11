#include <stdint.h>
#define CONFIG_ADDR 0xCF8
#define CONFIG_DATA 0xCFC

typedef struct {

	uint16_t v_id; // vendor id
	uint16_t d_id; // device id

} pci_info;

//refs
uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_config_read_long(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_config_write_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t data);
pci_info *pci_get_dev_credentials(uint8_t bus, uint8_t slot);
int pci_get_device_number(pci_info *dev);