#include "x86_desc.h"
#include "interrupt_handler.h"

#define SYSTEM_TRAP 0x80

extern void setup_system_handler(void){

  //set IDT entry 0x80 to point to system call set handler
  SET_IDT_ENTRY(idt[0x80], &system_call_handler);

  //set DPL to 3 to allow user-level programs to access system calls
  idt[0x80].dpl = 3;

  //trap gate to disable interrupts
  idt[0x80].reserved3 = 1;

  //set segment selector to kernel's code segment descriptor
  idt[0x80].seg_selector = KERNEL_CS;
}
