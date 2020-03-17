#define ASM
#include "x86_desc.h"

.text
# This context switch will IRET into USER mode, setting ESP, EBP, and EIP, along with proper segments and flags
.globl context_switch
context_switch:
  cli

  movl 4(%esp), %ecx # ecx has eip
  movl 8(%esp), %edx # edx has bottom_addr_of_page
  movl 12(%esp), %ebp # saving value of ebp in ebp

  # push values on stack in this order: ss, esp, eflags, cs, eip
  # push ss
  pushl $USER_DS # USER_DS

  # push esp
  pushl %edx

  # push eflags but with IF set
  pushfl
  popl %eax
  orl $0x200, %eax
  pushl %eax

  # push cs lower 2 bits is supposed to be new privilege level
  pushl $USER_CS # USER_CS

  # push eip
  pushl %ecx

  # perform actual context switch by popping into ss, esp, eflags, cs, eip registers
  iret

# This context switch will IRET into KERNEL mode, setting ESP, EBP, and EIP, along with proper segments and flags
.globl kernel_context_switch
kernel_context_switch:
  cli

  movl 4(%esp), %ecx # ecx has eip
  movl 8(%esp), %edx # edx has bottom_addr_of_page
  movl 12(%esp), %ebp # saving value of ebp in ebp

  # push values on stack in this order: ss, esp, eflags, cs, eip
  # push ss
  pushl $KERNEL_DS # KERNEL_DS

  # push esp
  pushl %edx

  # push eflags but with IF set
  pushfl
  popl %eax
  orl $0x200, %eax
  pushl %eax

  # push cs lower 2 bits is supposed to be new privilege level
  pushl $KERNEL_CS # KERNEL_CS

  # push eip
  pushl %ecx

  # perform actual context switch by popping into ss, esp, eflags, cs, eip registers
  iret

.globl execute_return
execute_return:
  movl 8(%esp), %ebp
  movl 4(%esp), %ebx
  xorl %eax, %eax
  movb 16(%esp), %al
  movl 12(%esp), %esp
  jmp *%ebx
