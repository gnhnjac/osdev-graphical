#include "pci.h"
#include "screen.h"
#include "irq.h"
#include "memory.h"
#include "rtl8139.h"
#include "low_level.h"

char rx_buffer[RX_BUF_SIZE + 16 + 1500]; // 1500 because wrap bit is active so to prevent overflow

rtl8139_dev_t rtl8139_dev;

// Four TXAD register, you must use a different one to send packet each time(for example, use the first one, second... fourth and back to the first)
uint8_t TSAD_array[4] = {0x20, 0x24, 0x28, 0x2C}; // address registers
uint8_t TSD_array[4] = {0x10, 0x14, 0x18, 0x1C}; // status registers

void install_nic()
{

	pci_info rtl8139 = {RTL_VID,RTL_DID};
	int dev_num = pci_get_device_number(&rtl8139);

	// activate bus mastering bit in the command register in the pci configuration of the device
	uint16_t cmd_reg = pci_config_read_word(0,dev_num,0,4);
	pci_config_write_word(0,dev_num,0,4,cmd_reg|2);

	// get the base i/o address of the device registers
	// IOAR at off 10h in the pci configuration
	uint32_t pci_bar_0 = pci_config_read_long(0,dev_num,0,0x10);

	rtl8139_dev.io_base = pci_bar_0 & (~0x3);

	// Set current TSAD because the device cycles between pairs of status/address registers to send data
    rtl8139_dev.tx_cur = 0;

	// get the mac address of the device
	printf("mac address: ");
	for (uint8_t i = 0; i < 6; i++)
	{
	    rtl8139_dev.mac_addr[i] = inb(rtl8139_dev.io_base + i); /*ioaddr is the base address obtainable from the PCI device configuration space.*/
	    if (i < 5)
	    	printf("%x:",rtl8139_dev.mac_addr[i]);
	   	else
	   		printf("%x\n",rtl8139_dev.mac_addr[i]);
	}

	// power on the device
	outb(rtl8139_dev.io_base+CFG1_REG,0);

	// reset the device to get default values in the registers
	outb(rtl8139_dev.io_base + CMD_REG, 0x10);
 	while((inb(rtl8139_dev.io_base + CMD_REG) & 0x10) != 0) { }

 	// // init buffer

 	// zero out the buffer
 	memset(rx_buffer,0,RX_BUF_SIZE+16+1500);

 	rtl8139_dev.rx_buffer = rx_buffer;
 	outl(rtl8139_dev.io_base + RBSTRT_REG, (uintptr_t)rtl8139_dev.rx_buffer); // send uint32_t memory location to RBSTART (0x30)


 	// // only accept the tok and rok irqs by setting the mask to only enable those 2
 	outw(rtl8139_dev.io_base + IMR_REG, ROK|TOK); // Sets the TOK and ROK bits high

 	// (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP
 	outl(rtl8139_dev.io_base + RCR_REG, 0xf | (1 << 7));

 	// Sets the RE and TE bits high
 	outb(rtl8139_dev.io_base + CMD_REG, 0x0C);

 	// init interrupt
	uint16_t ilr = pci_config_read_word(0,dev_num,0,0x3C)&0xFF; // interrupt line number which is the irq number (0-15)
 	irq_install_handler(ilr, rtl8139_handler);

}

// note: maximum len of packet is 1792
void send_packet(char *packet,int len)
{

	outl(rtl8139_dev.io_base+TSAD_array[rtl8139_dev.tx_cur],(uintptr_t)packet);
 	// 0-12 => size of packet
 	// 13 => own, 1 when not transmitting, 0 when transmitting
 	// 14 => tun
 	// 15 => tok
 	// ...

 	outl(rtl8139_dev.io_base+TSD_array[rtl8139_dev.tx_cur],len);
 	while(!inl(rtl8139_dev.io_base+TSD_array[rtl8139_dev.tx_cur])&(1<<13)) {} // while DMA is still transferring the data from buffer to internal buffer, wait.

 	if(++rtl8139_dev.tx_cur > 3)
 		rtl8139_dev.tx_cur = 0;

}

void rtl8139_handler(struct regs *r)
{

	uint16_t status = inw(rtl8139_dev.io_base + ISR_REG);

    if(status & TOK) { // if transmit ok received
        printf("Packet sent\n");
    }
    if (status & ROK) { // if received ok, handle packet
        printf("Received packet\n");
        receive_packet();
    }


    outw(rtl8139_dev.io_base + ISR_REG, ROK|TOK); // send to interrupt status register that transmit ok / receive ok to finish the handling of the interrupt


}

void receive_packet()
{



}