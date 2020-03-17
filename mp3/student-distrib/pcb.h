#ifndef PCB_H_
#define PCB_H_

#include "types.h"
#include "terminal.h"

#define PCB_MASK 0x1FFF
#define NUM_MAX_OPEN_FILES 8

// file struct
typedef struct file_t{
	int32_t file_ops_table_ptr;
	int32_t inode_num;
	int32_t file_position; // offset
	int32_t flags;
} file_t;

// struct for pcb in 4-8MB kernel page
typedef struct pcb_t {
	file_t file_array[NUM_MAX_OPEN_FILES];										// An array of file structs for each process
	uint32_t parent_num;                                      // 1-indexed PID of parent task, 0 if current task is root shell
	uint32_t parent_esp;																			// Value to set ESP to on calling halt (ESP of parent process)
	uint32_t current_esp;                                     // Value to set ESP to on switching to the task (stored in PIT interrupt, restored in later PIT interrupt)
	uint32_t parent_ebp;																			// Value to set EBP to on calling halt (EBP of parent process)
	uint32_t current_ebp;																			// Value to set EBP to on switching to the task (stored in PIT interrupt, restored in later PIT interrupt)
	uint32_t current_eip;																			// Value to set EIP to on switching to the task (stored in PIT interrupt, restored in later PIT interrupt)
	uint8_t* exec_ret_addr;																		// Value to set EIP to on calling halt (EIP of parent process)
	uint32_t rtc_count;																				// Current number of RTC ticks in current RTC read. Since only one RTC read at a time per process, just store number of 1024Hz ticks left in PCB
	uint8_t arg[TERMINAL_BUFFER_SIZE];												// Buffer containing the arguments to the process
	uint8_t num_char_in_arg;																	// Number of characters in argument buffer
	uint8_t terminal_index;                                   // What terminal this process is running on
	uint8_t is_user_mode;																			// Whether a PIT interrupt should return to user mode or kernel mode (useful for launching 2nd and 3rd terminal shells)
} pcb_t;


#endif
