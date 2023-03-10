#include "pci.h"
#include "screen.h"
#include "rtl8139.h"
#include "low_level.h"

char rx_buffer[8192 + 16];

void install_nic()
{

	pci_dev rtl8139 = {RTL_VID,RTL_DID};
	int dev_num = pci_get_device_number(&rtl8139);

	// activate bus mastering bit in the command register in the pci configuration of the device
	uint16_t cmd_reg = pci_config_read_word(0,dev_num,0,4);
	pci_config_write_word(0,dev_num,0,4,cmd_reg|2);

	// get the base i/o address of the device registers
	// IOAR at off 10h in the pci configuration
	// bit 8-31 BASE IO ADDR
	uint32_t ioaddr = pci_config_read_word(0,dev_num,0,0x10) >> 8; // first 8 bits
	ioaddr |= pci_config_read_word(0,dev_num,0,0x10+2)<<8; // last 16 bits

	// set interrupt to interrupt number 48 to not override any existing ones in the idt
	uint16_t ilr_ipr = pci_config_read_word(0,dev_num,0,0x3C);
	ilr_ipr = (ilr_ipr&0xFF00) | 48;
	pci_config_write_word(0,dev_num,0,0x3C,ilr_ipr);

	// power on the device
	outb(ioaddr+CFG1_REG,0);

	// reset the device to avoid junk in the registers
	printf("%x\n",ioaddr);
	outb(ioaddr + CMD_REG, 0x10);
 	while((inb(ioaddr + CMD_REG) & 0x10) != 0) { }

 	// // init buffer
 	// outl(ioaddr + RBSTRT_REG, (uintptr_t)rx_buffer); // send uint32_t memory location to RBSTART (0x30)


 	// // only accept the tok and rok irqs by setting the mask to only enable those 2
 	// outw(ioaddr + IMR_REG, 0x0005); // Sets the TOK and ROK bits high


 	// // to understand later
 	// outl(ioaddr + 0x44, 0xf | (1 << 7)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP
 	// outb(ioaddr + 0x37, 0x0C); // Sets the RE and TE bits high

}