#include "gdt.h"
#include "memory.h"

// Global Descriptor Table (GDT)
static struct gdt_descriptor _gdt [MAX_DESCRIPTORS];
 
//! gdtr data
struct gdtr	_gdtr;

extern void gdt_load(void);

//! installs gdtr
void gdt_install () {

	//! set up gdtr
	_gdtr.m_limit = (sizeof (struct gdt_descriptor) * MAX_DESCRIPTORS)-1;
	_gdtr.m_base = (uint32_t)&_gdt[0];
 
	//! set null descriptor
	gdt_set_descriptor(0, 0, 0, 0, 0);
 
	//! set default code descriptor
	gdt_set_descriptor (1,0,0xffffffff,
		I86_GDT_DESC_READWRITE|I86_GDT_DESC_EXEC_CODE|I86_GDT_DESC_CODEDATA|I86_GDT_DESC_MEMORY,
		I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);
 
	//! set default data descriptor
	gdt_set_descriptor (2,0,0xffffffff,
		I86_GDT_DESC_READWRITE|I86_GDT_DESC_CODEDATA|I86_GDT_DESC_MEMORY,
		I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	//! set default user mode code descriptor
	gdt_set_descriptor (3,0,0xffffffff,
		I86_GDT_DESC_READWRITE|I86_GDT_DESC_EXEC_CODE|I86_GDT_DESC_CODEDATA|
		I86_GDT_DESC_MEMORY|I86_GDT_DESC_DPL,
			I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	//! set default user mode data descriptor
	gdt_set_descriptor (4,0,0xffffffff,
		I86_GDT_DESC_READWRITE|I86_GDT_DESC_CODEDATA|I86_GDT_DESC_MEMORY|
		I86_GDT_DESC_DPL,
			I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	gdt_load();
}

//! Setup a descriptor in the Global Descriptor Table
void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit, uint8_t access, uint8_t grand)
{
	if (i > MAX_DESCRIPTORS)
		return;
 
	//! null out the descriptor
	memset ((void*)&_gdt[i], 0, sizeof (struct gdt_descriptor));
 
	//! set limit and base addresses
	_gdt[i].baseLo	= base & 0xffff;
	_gdt[i].baseMid	= (base >> 16) & 0xff;
	_gdt[i].baseHi	= (base >> 24) & 0xff;
	_gdt[i].limit	= limit & 0xffff;
 
	//! set flags and grandularity bytes
	_gdt[i].flags = access;
	_gdt[i].grand = (limit >> 16) & 0x0f;
	_gdt[i].grand |= grand & 0xf0;
 
}