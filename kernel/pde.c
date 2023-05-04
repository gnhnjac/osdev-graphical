#include "pde.h"

void pd_entry_add_attrib (pd_entry* e, uint32_t attrib)
{
	*e |= attrib;
}
void pd_entry_del_attrib (pd_entry* e, uint32_t attrib)
{
	*e &= ~attrib;
}
void pd_entry_set_frame (pd_entry* e, void *physical_addr)
{
	pd_entry_del_attrib(e,I86_PDE_FRAME);
	*e |= (((uint32_t)physical_addr&0x7FFFF)<<12);
}
bool pd_entry_is_present (pd_entry e)
{

	return e&I86_PDE_PRESENT;

}
bool pd_entry_is_user (pd_entry e)
{

	return e&I86_PDE_USER;

}

bool pd_entry_is_4mb (pd_entry e)
{

	return e&I86_PDE_4MB;

}
bool pd_entry_is_writable (pd_entry e)
{
	return e&I86_PDE_WRITABLE;
}
void *pd_entry_pfn (pd_entry e)
{
	return (void *)(e&I86_PDE_FRAME >> 12);
}
void pd_entry_enable_global (pd_entry *e)
{
	pd_entry_add_attrib(e,I86_PDE_CPU_GLOBAL);
	pd_entry_add_attrib(e,I86_PDE_LV4_GLOBAL);
}