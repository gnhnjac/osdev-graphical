#include "pci.h"
#include "low_level.h"
#include "heap.h"
#include "std.h"

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address; // register offset
    uint32_t lbus  = (uint32_t)bus; // pci bus number
    uint32_t lslot = (uint32_t)slot; // device number
    uint32_t lfunc = (uint32_t)func; // device function
    uint16_t tmp = 0; // the word we want to read
 
    // Create configuration address
    // BUS_NUM(16-23)|DEV_NUM(11-15)|FUNC_NUM(8-10)|REG_OFF(0-7)
    // last number is a constant for reserved stuff
    // first 2 bits of offset is always 0 by design so it'll be 32 bit aligned
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
    outl(CONFIG_ADDR, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    // (shift by word if want to read 2nd 16 bit part of 32 bit register and that's done by masking 2nd bit of offset).
    tmp = (uint16_t)((inl(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

uint32_t pci_config_read_long(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address; // register offset
    uint32_t lbus  = (uint32_t)bus; // pci bus number
    uint32_t lslot = (uint32_t)slot; // device number
    uint32_t lfunc = (uint32_t)func; // device function
    uint16_t tmp = 0; // the word we want to read
 
    // Create configuration address
    // BUS_NUM(16-23)|DEV_NUM(11-15)|FUNC_NUM(8-10)|REG_OFF(0-7)
    // last number is a constant for reserved stuff
    // first 2 bits of offset is always 0 by design so it'll be 32 bit aligned
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
    outl(CONFIG_ADDR, address);
    // Read in the data
    return inl(CONFIG_DATA);
}


void pci_config_write_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t data) {
    uint32_t address; // register offset
    uint32_t lbus  = (uint32_t)bus; // pci bus number
    uint32_t lslot = (uint32_t)slot; // device number
    uint32_t lfunc = (uint32_t)func; // device function
    uint16_t tmp = 0; // the word we want to read
 	
    uint16_t other_word;
    uint32_t write_data;

    if(offset&2)
    {
    	// because we can only write 32 bit segments through the CONFIG_DATA i/o register port we want to preserve the other word in the 32 bit.
    	other_word = pci_config_read_word(bus,slot,func,offset-2);
    }
    else
    {
    	other_word = pci_config_read_word(bus,slot,func,offset+2);
    }

    // Create configuration address
    // BUS_NUM(16-23)|DEV_NUM(11-15)|FUNC_NUM(8-10)|REG_OFF(0-7)
    // last number is a constant for reserved stuff
    // first 2 bits of offset is always 0 by design so it'll be 32 bit aligned
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    // Write out the address
    outl(CONFIG_ADDR, address);

    if(offset&2)
    {

    	write_data = (uint32_t)other_word|((uint32_t)data<<16);
    }
    else
    {
    	write_data = (uint32_t)data|((uint32_t)other_word<<16);
    }

    // Write the data
    outl(CONFIG_DATA,write_data);
}

// mallocs the return
pci_info *pci_get_dev_credentials(uint8_t bus, uint8_t slot) {
    pci_info *dev = (pci_info *)kmalloc(4);
    dev->d_id = 0;
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    if ((dev->v_id=pci_config_read_word(bus, slot, 0, 0)) != 0xFFFF) {
       dev->d_id = pci_config_read_word(bus, slot, 0, 2);
    }
    return dev;
}

int pci_get_device_number(pci_info *dev)
{
	int n = 0;
	for (; n < 256; n++)
	{

		pci_info *creds = pci_get_dev_credentials(0,n);
		if (dev->v_id==creds->v_id && dev->d_id==creds->d_id)
			break;
		kfree(creds);

	}
	if (n == 256)
		return STDERR;

	return n;

}