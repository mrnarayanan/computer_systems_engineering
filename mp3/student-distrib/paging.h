#ifndef PAGING_H
#define PAGING_H

#include "types.h"

//maximum number of processes
#define NUM_MAX_PROCESSES 6

// number of entries in page directory and page table
#define NUM_ENTRIES  1024

// declare global page directory
extern uint32_t page_directory_array[NUM_MAX_PROCESSES][NUM_ENTRIES];

// declare global page table mapping from virtual 0MB to physical 0MB
extern uint32_t first_page_table[NUM_ENTRIES];

// declare global page table mapping from virtual 256MB to physical 0MB
extern uint32_t video_mem_page_table[NUM_MAX_PROCESSES][NUM_ENTRIES];

/* loadPageDirectory
 * inputs: unsigned long * - pointer to page directory
 * outputs: none
 * return value: none
 * side effects: set cr3 (loads page directory)
 */
extern void loadPageDirectory(uint32_t * arg);

/* setPageSize
 * inputs: none
 * outputs: none
 * return value: none
 * side effects: sets cr4 (PSE and clears PAE)
 */
extern void setPageSize();

/* enablePaging
 * inputs: none
 * outputs: none
 * return value: none
 * side effects: set cr0 (enables paging)
 */
extern void enablePaging();

/* initPaging
 * inputs: unsigned long * - pointer to page directory
 * outputs: none
 * return value: none
 * side effects: inits a page dir, loads it, enables paging
 */
extern void initPaging();

#endif
