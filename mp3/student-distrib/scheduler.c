#include "scheduler.h"

static uint32_t terminal_iter = 0;

#define PIT_IRQ_PORT 0x40
#define PIT_CMD_PORT 0x43
#define PIT_INT_MODE 0x36

/* schedule
 * DESCRIPTION: implements round robin scheduling among running processes
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECTS: modifies values in PCB, changes page directory
 */
void schedule(void)
{
  // Save esp/ebp - save current process esp and ebp in PCB
  uint32_t next_ebp, next_esp, next_eip;
  int32_t pid;
  pcb_t *my_pcb = getCurrentProcessPCB();
  pcb_t *next_pcb;
  asm volatile("movl %%ebp, %0" : "=r" (my_pcb->current_ebp));
  asm volatile("movl %%esp, %0" : "=r" (my_pcb->current_esp));

  my_pcb->current_eip = (uint32_t) &&EIP_RETURN;
  my_pcb->is_user_mode = 0;

  // Switch process paging
  terminal_iter = (terminal_iter + 1) % NUM_TERMINALS;

  pid = (int32_t) terminals[terminal_iter].active_process;
  while (pid == -1)
  {
    terminal_iter = (terminal_iter + 1) % NUM_TERMINALS;
    pid = terminals[terminal_iter].active_process;
  }

  loadPageDirectory(page_directory_array[pid - 1]);

  // Set TSS
  tss.esp0 = get_kernel_stack_bottom(pid);

  // Restore next process’ esp/ebp
  next_pcb = getProcessPCB( (uint32_t) pid);
  next_ebp = next_pcb->current_ebp;
  next_esp = next_pcb->current_esp;
  next_eip = next_pcb->current_eip;
  curr_process = pid;
  if(next_pcb->is_user_mode)
  {
      send_eoi(0);
      context_switch(next_eip, next_esp, next_ebp);
  }
  else
  {
    kernel_context_switch(next_eip, next_esp, next_ebp);
  }

  EIP_RETURN:;
  return;
}

/* pit_init
 * DESCRIPTION: initializes the PIT
 * INPUTS: none
 * OUTPUTS: issues commands to set PIT frequencies
 * RETURN VALUE: none
 * SIDE EFFECTS: PIT frequency set to square wave with frequency of 100 Hz
 */
//Most of this code is from OSDev
void pit_init(void)
{
    //set the frequency to 100Hz = 1 interrupt every 100ms
    uint16_t rate = 1193180 / 100;         //values from OSDEV
    outb(PIT_INT_MODE, PIT_CMD_PORT);
    outb(rate & 0xFF, PIT_IRQ_PORT);
    outb(rate >> 8, PIT_IRQ_PORT);
}
