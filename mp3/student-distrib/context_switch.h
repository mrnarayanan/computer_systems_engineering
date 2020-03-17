#ifndef CONTEXT_SWITCH_H
#define CONTEXT_SWITCH_H

extern void context_switch(uint32_t eip, uint32_t esp, uint32_t ebp);
extern void kernel_context_switch(uint32_t eip, uint32_t esp, uint32_t ebp);

extern uint32_t get_exec_ret_addr(void);

extern void execute_return(uint8_t* execute_return_addr, uint32_t, uint32_t, uint8_t status);

extern uint32_t save_parent_esp();

extern uint32_t save_parent_ebp();

extern void restore_parent_ebp(uint32_t parent_ebp);

#endif //CONTEXT_SWITCH_H
