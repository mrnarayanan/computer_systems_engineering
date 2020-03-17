#define ASM
#include "interrupt_handler.h"

.text

.globl do_halt
/* do_halt
 * DESCRIPTION: assembly wrapper for halt
 * INPUTS: status
 * OUTPUTS: sets correct syscall number (1) and calls INT 0x80
 * RETURN VALUE: int 0 if successful
 * SIDE EFFECTS: changes curr_process
 */
do_halt:
  PUSHL	%EBX
  MOVL	$1,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

.globl do_execute
/* do_execute
 * DESCRIPTION: assembly wrapper for execute
 * INPUTS: command as character array
 * OUTPUTS: sets correct syscall number (2) and calls INT 0x80
 * RETURN VALUE: int 0 if success, int -1 if fail
 * SIDE EFFECTS: modifies PCB
 */
do_execute:
  PUSHL	%EBX
  MOVL	$2,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

.globl do_read
/* do_read
 * DESCRIPTION: assembly wrapper for read
 * INPUTS: file descriptor index, buffer array pointer, number of bytes to read
 * OUTPUTS: sets correct syscall number (3) and calls INT 0x80
 * RETURN VALUE: number of bytes read, -1 if failure
 * SIDE EFFECTS: none
 */
do_read:
  PUSHL	%EBX
  MOVL	$3,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

.globl do_write
/* do_write
 * DESCRIPTION: assembly wrapper for write
 * INPUTS: file descriptor index, buffer array pointer, number of bytes to write
 * OUTPUTS: sets correct syscall number (4) and calls INT 0x80
 * RETURN VALUE: number of bytes written, -1 if failure
 * SIDE EFFECTS: none
 */
do_write:
  PUSHL	%EBX
  MOVL	$4,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

.globl do_open
/* do_open
 * DESCRIPTION: assembly wrapper for open
 * INPUTS: filename as character array
 * OUTPUTS: sets correct syscall number (5) and calls INT 0x80
 * RETURN VALUE: 0 on success, -1 on fail
 * SIDE EFFECTS: populates file array
 */
do_open:
  PUSHL	%EBX
  MOVL	$5,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

.globl do_close
/* do_close
 * DESCRIPTION: assembly wrapper for close
 * INPUTS: file descriptor index
 * OUTPUTS: sets correct syscall number (6) and calls INT 0x80
 * RETURN VALUE: 0 on success, -1 on failure
 * SIDE EFFECTS: modifies file array
 */
do_close:
  PUSHL	%EBX
  MOVL	$6,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

.globl do_getargs
/* do_getargs
 * DESCRIPTION: assembly wrapper for getargs
 * INPUTS: pointer to buffer, number of bytes
 * OUTPUTS: sets correct syscall number (7) and calls INT 0x80
 * RETURN VALUE: 0 on success, -1 on fail
 * SIDE EFFECTS: reads from PCB
 */
do_getargs:
  PUSHL	%EBX
  MOVL	$7,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

.globl do_vidmap
/* do_vidmap
 * DESCRIPTION: assembly wrapper for vidmap
 * INPUTS: double pointer to screen_start
 * OUTPUTS: sets correct syscall number (8) and calls INT 0x80
 * RETURN VALUE: 0 on success, -1 on failure
 * SIDE EFFECTS: modifies page directory
 */
do_vidmap:
  PUSHL	%EBX
  MOVL	$8,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

.globl do_set_handler
/* do_set_handler
 * DESCRIPTION: assembly wrapper for set_handler
 * INPUTS:
 * OUTPUTS: sets correct syscall number (9) and calls INT 0x80
 * RETURN VALUE:
 * SIDE EFFECTS:
 */
do_set_handler:
  PUSHL	%EBX
  MOVL	$9,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

.globl do_sigreturn
/* do_sigreturn
 * DESCRIPTION: assembly wrapper for sigreturn
 * INPUTS:
 * OUTPUTS: sets correct syscall number (10) and calls INT 0x80
 * RETURN VALUE:
 * SIDE EFFECTS:
 */
do_sigreturn:
  PUSHL	%EBX
  MOVL	$10,%EAX
  MOVL	8(%ESP),%EBX
  MOVL	12(%ESP),%ECX
  MOVL	16(%ESP),%EDX
  INT	  $0x80
  POPL	%EBX
  RET

# Declare the functions in this file as global
.text
.globl rtc_handler
/*
 * rtc_handler
 *   DESCRIPTION: Handles RTC interrupt in method specified by documentation
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sends EOI to PIC and does whatever test_interrupts does
 *                   All registers should be saved and restored
 */
rtc_handler:
    PUSHAL                      //Save all the registers
#    call test_interrupts                //Needs no arguments, push nothing to stack
    call rtc_int
    pushl $8                    //Assume pass IRQ number to send eoi
    call send_eoi               //Send the end of interrupt signal
    addl $4, %esp               //Pop IRQ argument from stack
    POPAL                       //Restore all the registers
    IRET                        //Return from interrupt handler

.globl key_handler
/*
 * key_handler
 *   DESCRIPTION: Is a wrapper function for a c function to handle key press or release
 *   INPUTS: none
 *   OUTPUTS: the key pressed to the screen
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes to the screen, but all registers should be saved and restored
 */
key_handler:
    PUSHAL                      //Save all registers
    call handle_keypress        //Do all processing in C
    pushl $1                    //Pass IRQ number to send EOI
    call send_eoi
    addl $4, %esp               //Pop argument from stack
    POPAL                       //Restore all registers
    IRET                        //Return from interrupt handler

/*
 * trap_handler
 *   DESCRIPTION: Assembly wrapper for a c function to handle when a trap happens
 *   INPUTS: currently none
 *   OUTPUTS: function called writes to screen
 *   RETURN VALUE: currently none
 *   SIDE EFFECTS: function called writes to screen
 */
.globl trap_handler
trap_handler:
    PUSHAL                      //Save all registers
    PUSHL $SYSCALL_MESSAGE
    call printf            //Dummy method, prints that trap was called
    POPL %eax
    POPAL
    IRET

/*
 * scheduler_handler
 *   DESCRIPTION: Assembly wrapper for schedule()
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls schedule() and sends eoi afterwards
 */
.globl scheduler_handler
scheduler_handler:
    PUSHAL                      //Save all registers
    call schedule               //Do all processing in C
    pushl $0                    //Pass IRQ number to send EOI for PIT
    call send_eoi
    addl $4, %esp               //Pop argument from stack
    POPAL                       //Restore all registers
    IRET

/*
 * system_call_handler
 *   DESCRIPTION: Assembly wrapper for a c function to handle a system call
 *   INPUTS: %eax - syscall number, %ebx - 1st arg of syscall, %ecx - 2nd arg of syscall, %edx - 3rd arg of syscall
 *   OUTPUTS: The output of the corresponding syscall in question
 *   RETURN VALUE: In %eax, depending on the syscall in question, some syscalls do not return values
 */
.globl system_call_handler
system_call_handler:
/* Save all registers first */
    push %gs
    push %fs
    push %es
    push %ds
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %esp
    pushl %edx
    pushl %ecx
    pushl %ebx
    # Following code courtesy of OSDEV. Informs of all registers that need saving
    # system call number already in %eax
    # call dispatcher that will use arg in %eax to call specified system call
    call syscall_dispatcher
/* Restores registers */
    popl %ebx
    popl %ecx
    popl %edx
    popl %esp
    popl %esi
    popl %edi
    popl %ebp
    pop %ds
    pop %es
    pop %fs
    pop %gs
    IRET

/*
 * syscall_dispatcher
 *   DESCRIPTION: Calls correct system call out of the possible 10 based on system call number
 *   INPUTS: %eax - syscall number
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fail
 *   SIDE EFFECTS: jumps to system call in jump table
 */
syscall_dispatcher:
    cmpl  $0, %eax
    je    fail
    cmpl  $10, %eax # valid cmd options are between 1-10
    ja    fail
    jmp   *jump_table(,%eax,4)
    fail:
    movl  $-1, %eax
    ret

jump_table:
.long   0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

.data
SYSCALL_MESSAGE:
  .string "System trap was called\n"
