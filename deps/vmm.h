#pragma once
#include "pde.h"
#include "pte.h"

//! virtual address
typedef uint32_t virtual_addr;
 
//! i86 architecture defines 1024 entries per table--do not change
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff) // get dir index part of vaddr
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff) // get table index part of vaddr
#define FRAME_OFFSET(x) (((x)) & 0xfff) // get frame offset part of vaddr
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff) // get phys address from pte pointer.

//! page table represents 4mb address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000

//! directory table represents 4gb address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

//! page sizes are 4k
#define PAGE_SIZE 4096

#define K_REAL_BASE 0x100000 // real kernel base. (1mb)

#define K_VIRT_BASE 0xc0000000 // virtual kernel base. (3gb)
 
//! page table
typedef struct {
 
	pt_entry m_entries[PAGES_PER_TABLE];
} ptable;
 
//! page directory
typedef struct {
 
	pd_entry m_entries[PAGES_PER_DIR];
} pdirectory;

extern void vmmngr_flush_tlb_entry (virtual_addr addr);
//refs
//! current directory table (global);
bool vmmngr_alloc_page (pt_entry* e, uint32_t pte_flags);
void vmmngr_free_page (pt_entry* e);
pt_entry* vmmngr_ptable_lookup_entry (ptable* p,virtual_addr addr);
pd_entry* vmmngr_pdirectory_lookup_entry (pdirectory* p, virtual_addr addr);
bool vmmngr_switch_pdirectory (pdirectory* dir);
void vmmngr_set_pdirectory_ptr(pdirectory *dir);
pdirectory* vmmngr_get_directory ();
pdirectory* vmmngr_create_pdir ();
void vmmngr_free_pdir(pdirectory *pdir);
void * vmmngr_virt2phys(void *virt);
void * vmmngr_virt2phys_pdir(pdirectory *pdir, void *virt);
void vmmngr_alloc_virt(pdirectory *dir, void *virt, uint32_t pde_flags, uint32_t pte_flags);
bool vmmngr_check_virt_present(pdirectory *dir, void *virt);
void vmmngr_free_virt(pdirectory *dir, void *virt);
void vmmngr_map_page (pdirectory *dir, void* phys, void* virt, uint32_t pde_flags, uint32_t pte_flags);
void vmmngr_mmap_virt2virt(pdirectory *orig, pdirectory *new, void* orig_virt, void* new_virt, uint32_t pde_flags, uint32_t pte_flags);
void vmmngr_unmap_virt(pdirectory *dir, void *virt);
void vmmngr_pdirectory_clear(pdirectory *dir);
void vmmngr_ptable_clear(ptable *table);
void vmmngr_initialize ();