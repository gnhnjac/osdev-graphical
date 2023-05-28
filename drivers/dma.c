#include "dma.h"
#include "low_level.h"

void dma_set_address(uint8_t channel, uint8_t low, uint8_t high) {

	// set the dma address to write or read from

	if ( channel > 8 )
		return;

	unsigned short port = 0;
	switch ( channel ) {

		case 0: {port = DMA0_CHAN0_ADDR_REG; break;}
		case 1: {port = DMA0_CHAN1_ADDR_REG; break;}
		case 2: {port = DMA0_CHAN2_ADDR_REG; break;}
		case 3: {port = DMA0_CHAN3_ADDR_REG; break;}
		case 4: {port = DMA1_CHAN4_ADDR_REG; break;}
		case 5: {port = DMA1_CHAN5_ADDR_REG; break;}
		case 6: {port = DMA1_CHAN6_ADDR_REG; break;}
		case 7: {port = DMA1_CHAN7_ADDR_REG; break;}
	}

	outb(port, low);
	outb(port, high);
}

void dma_set_count(uint8_t channel, uint8_t low, uint8_t high) {

	// set how much to read/write (64k max)

	if ( channel > 8 )
		return;

	unsigned short port = 0;
	switch ( channel ) {

		case 0: {port = DMA0_CHAN0_COUNT_REG; break;}
		case 1: {port = DMA0_CHAN1_COUNT_REG; break;}
		case 2: {port = DMA0_CHAN2_COUNT_REG; break;}
		case 3: {port = DMA0_CHAN3_COUNT_REG; break;}
		case 4: {port = DMA1_CHAN4_COUNT_REG; break;}
		case 5: {port = DMA1_CHAN5_COUNT_REG; break;}
		case 6: {port = DMA1_CHAN6_COUNT_REG; break;}
		case 7: {port = DMA1_CHAN7_COUNT_REG; break;}
	}

	outb(port, low);
	outb(port, high);
}