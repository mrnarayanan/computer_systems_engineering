#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include "types.h"
#include "lib.h"
#include "vfs.h"
#include "rtc.h"
#include "paging.h"
#include "x86_desc.h"
#include "context_switch.h"
#include "terminal.h"
#include "pcb.h"

#define OPEN 0
#define READ 1
#define WRITE 2
#define CLOSE 3
#define INACTIVE 0
#define ACTIVE 1
#define P_PROCESS_BASE 0x800000 //P for physical
#define P_PROCESS_OFFSET 0x2000
#define V_PROGRAM_BASE 0x8048000 //V for virtual
#define MB_256 0x10000000
#define MB_128 0x8000000
#define MB_4 0x400000
#define KB_4 0x1000
#define B_4 4
#define PCB_MASK 0x1FFF

/* global to keep track of the current process
 */
extern uint32_t curr_process;

/* bitmap for keeping track of what num_processes
 * use which pages
 */
extern uint8_t active[NUM_MAX_PROCESSES];

extern int32_t open(const uint8_t* filename);

extern int32_t close(int32_t fd);

extern int32_t read(int32_t fd, void* buf, int32_t nbytes); // uint8_t* instead of void*

extern int32_t write(int32_t fd, const void* buf, int32_t nbytes); // uint8_t* instead of void*

extern int32_t halt(uint8_t status);

extern int32_t execute(const uint8_t* command);

extern int32_t getargs(uint8_t* buf, int32_t nbytes);

extern int32_t vidmap(uint8_t** screen_start);

extern int32_t set_handler(int32_t signum, void* handler); // uint8_t* instead of void*

extern int32_t sigreturn(void);

//Helper function to get current PCB
pcb_t * getCurrentProcessPCB();

//Helper function to get PCB associated with any process
pcb_t * getProcessPCB(uint32_t pid);

//Helper function to get the address of the bottom of any process' kernel stack
uint32_t get_kernel_stack_bottom(uint32_t pid);

// typedefs for function pointers for close, read, write
typedef int32_t (*CLS)(int32_t);
typedef int32_t (*RD)(int32_t, void*, int32_t);
typedef int32_t (*WRT)(int32_t, const void*, int32_t);

#endif
