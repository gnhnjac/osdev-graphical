#include "pmm.h"
#include "vmm.h"
#include "memory.h"
#include "screen.h"

//! current directory table (global)
pdirectory*	_cur_directory=0;
 

bool vmmngr_alloc_page (pt_entry* e) {
 
	//! allocate a free physical frame
	void* p = pmmngr_alloc_block ();
	if (!p)
		return false;
 
	//! map it to the page
	pt_entry_set_frame (e, (void *)p);
	pt_entry_add_attrib (e, I86_PTE_PRESENT);
 
	return true;
}

void vmmngr_free_page (pt_entry* e) {
 
	void* p = (void*)pt_entry_pfn (*e);
	if (p) // if not zero because we always want the first block of memory to be occupied for null exceptions.
		pmmngr_free_block (p);
 
	pt_entry_del_attrib (e, I86_PTE_PRESENT);
}

pt_entry* vmmngr_ptable_lookup_entry (ptable* p,virtual_addr addr) {
 
	if (p)
		return &p->m_entries[ PAGE_TABLE_INDEX (addr) ];
	return 0;
}

pd_entry* vmmngr_pdirectory_lookup_entry (pdirectory* p, virtual_addr addr) {
 
	if (p)
		return &p->m_entries[ PAGE_DIRECTORY_INDEX (addr) ];
	return 0;
}

bool vmmngr_switch_pdirectory (pdirectory* dir) {
 
	if (!dir)
		return false;
 
	_cur_directory = dir;
	pmmngr_load_PDBR ((void *)_cur_directory);
	return true;
}

pdirectory* vmmngr_get_directory () {
 
	return _cur_directory;
}

extern void vmmngr_flush_tlb_entry (virtual_addr addr);

void * vmmngr_virt2phys(void *virt)
{
	//! get page directory
   pdirectory* pageDirectory = vmmngr_get_directory ();

   //! get page directory entry
   pd_entry* pageDirectoryEntry = vmmngr_pdirectory_lookup_entry(pageDirectory,(virtual_addr)virt);
   if (!pd_entry_is_present(*pageDirectoryEntry))
   	return 0;

   //! get page table
   ptable* pageTable = (ptable*) pd_entry_pfn(*pageDirectoryEntry);

   //! get page entry
   pt_entry* pageEntry = vmmngr_ptable_lookup_entry(pageTable,(virtual_addr)virt);
   if (!pt_entry_is_present(*pageEntry))
   	return 0;
   printf("%x",pt_entry_pfn(*pageEntry)+FRAME_OFFSET((uint32_t)virt));
   // return the frame + offset
   return pt_entry_pfn(*pageEntry) + FRAME_OFFSET((uint32_t)virt);

}

void vmmngr_map_page (void* phys, void* virt) {

   //! get page directory
   pdirectory* pageDirectory = vmmngr_get_directory ();

   //! get page table
   pd_entry* e = vmmngr_pdirectory_lookup_entry(pageDirectory,(virtual_addr)virt);

   if (!pd_entry_is_present(*e)) {
		//! page table not present, allocate it
   	ptable* table = (ptable*) pmmngr_alloc_block ();
   	if (!table)
      	return;

      	//! clear page table
     	vmmngr_ptable_clear(table);

   	//! map in the table (Can also just do *entry |= 3) to enable these bits
   	pd_entry_add_attrib (e, I86_PDE_PRESENT);
   	pd_entry_add_attrib (e, I86_PDE_WRITABLE);
   	pd_entry_set_frame (e, (void *)table);
   }

   //! get table
   ptable* table = (ptable*) pd_entry_pfn(*e);

   //! get page
   pt_entry* page = vmmngr_ptable_lookup_entry(table,(virtual_addr)virt);

   //! map it in (Can also do (*page |= 3 to enable..)
   pt_entry_set_frame ( page, (void *) phys);
   pt_entry_add_attrib ( page, I86_PTE_PRESENT);
}

void vmmngr_pdirectory_clear(pdirectory *dir)
{

	memset((void *)dir,0,1024*4);

}

void vmmngr_ptable_clear(ptable *table)
{

	memset((void *)table,0,1024*4);

}

void vmmngr_mmap(uint32_t frame_start, uint32_t virt_start, uint32_t range) // range is in 4kb units
{
	for (uint32_t i=0, frame=frame_start, virt=virt_start; i<range; i++, frame+=4096, virt+=4096) 
	{
 		vmmngr_map_page((void *)frame,(void *)virt);
	}
}

void vmmngr_initialize () {
	//! allocate our page directory
	pdirectory* dir = (pdirectory*) pmmngr_alloc_block ();
	if (!dir)
		return;
	vmmngr_pdirectory_clear (dir); // clear it out
	// store current directory
	_cur_directory = dir;

	//! identity map 0-16mb->0-16mb
	vmmngr_mmap(0x0,0x00000000,1024*4);
 
	//! virtual map 3gb-3gb+16mb->0-16mb
	vmmngr_mmap(0x100000,K_VIRT_BASE,1024*4);

	// switch to our page directory
	vmmngr_switch_pdirectory (dir);
	//! enable paging
	pmmngr_paging_enable (true);
}