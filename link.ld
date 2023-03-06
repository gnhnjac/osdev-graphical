_KERNEL_BASE_ = 0x10000;
ENTRY(_KERNEL_BASE_)
SECTIONS
{
  . = _KERNEL_BASE_;
  .text.start (_KERNEL_BASE_) : {
    kernel/kernel_entry.o( .text )
  }

  .text : ALIGN(4096) {
_TEXT_START_ = .;
    *(.text)
_TEXT_END_ = .;
  }

  .data : ALIGN(4096) {
_DATA_START_ = .;
    *(.data)
_DATA_END_ = .;
  }

  .bss : ALIGN(4096) {
_BSS_START_ = .;
    *(.bss)
_BSS_END_ = .;
  }
}