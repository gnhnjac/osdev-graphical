#include "pte.h"

void pt_entry_add_attrib (pt_entry* e, uint32_t attrib)
{
	*e |= attrib;
}
void pt_entry_del_attrib (pt_entry* e, uint32_t attrib)
{
	*e &= ~attrib;
}
void pt_entry_set_frame (pt_entry* e, void *physical_addr)
{
	pt_entry_del_attrib(e,I86_PTE_FRAME);
	*e |= (uint32_t)physical_addr&I86_PTE_FRAME;
}
bool pt_entry_is_present (pt_entry e)
{

	return e&I86_PTE_PRESENT;

}
bool pt_entry_is_writable (pt_entry e)
{

	return e&I86_PTE_WRITABLE;

}
void *pt_entry_pfn (pt_entry e)
{
	return (void *)(e&I86_PTE_FRAME);
}