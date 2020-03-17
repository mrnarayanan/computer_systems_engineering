#include "paging.h"

#define VMEM_BASE 184
#define VMEM_TOP 187

//define page directories array, align each to 4kB
uint32_t page_directory_array[NUM_MAX_PROCESSES][NUM_ENTRIES] __attribute__((aligned(4096)));

//0-4MB page table array, align each to 4kB
uint32_t first_page_table[NUM_ENTRIES] __attribute__((aligned(4096)));

//128-132MB page table array for vidmap, align each to 4kB
uint32_t video_mem_page_table[NUM_MAX_PROCESSES][NUM_ENTRIES] __attribute__((aligned(4096)));


/* initPaging
 * description: The main function to set up everything needed for paging
 * inputs: unsigned long * - pointer to page directory
 * outputs: none
 * return value: noned
 * side effects: inits a page dir, loads it, enables paging
 */
void initPaging()
{
	//initialize each entry to not present
	int i, j;

	for (i = 0; i < NUM_MAX_PROCESSES; i++){
		for(j = 0; j < NUM_ENTRIES; j++)
		{
		  // This sets the following flags to the pages:
		  //   Supervisor: Only kernel-mode can access them
		  //   Write Enabled: It can be both read from and written to
		  //   Not Present: The page table is not present
		  page_directory_array[i][j] = 0x00000002;
		}
	}

	//initialize first page table containing video memory
	for(i = 0; i < NUM_ENTRIES; i++)
	{
		// As the address is page aligned, it will always leave 12 bits zeroed.
		// Those bits are used by the attributes ;)
		if (i >= VMEM_BASE && i <= VMEM_TOP){
			first_page_table[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
		}
		//map other virtual address as not present
		else{
			first_page_table[i] = (i * 0x1000) | 2; // attributes: supervisor level, read/write, not present.
		}
	}


	// set first entry in first page directory to point to first page table
	page_directory_array[0][0] = ((unsigned int) first_page_table) | 3;

	for (i = 0; i < NUM_MAX_PROCESSES; i++){
		//Try setting the first page table in each page directory to first page table
		//Hopefully allows for reading/writing to video memory
		page_directory_array[i][0] = ((unsigned int) first_page_table) | 3;

		// set second entry in page directories to point to second page table
		// 7 for supervisor, R/W and present bit set
		// 8 for 4MB page size for that entry
		// 4 for starting at address 0x400000 (corresponds to 4MB starting location for kernel page)
		page_directory_array[i][1] = 0x00400083;

		//map virtual 128 MB to physical 8MB + (i * 4MB) where i is the page directory #
		//32 because it corresponds with 128 MB virtual address
		page_directory_array[i][32] = 0x00800087 + i * 0x00400000;
	}

	// set control registers to initialize paging
	loadPageDirectory(page_directory_array[0]); // set cr3 to point to first page directory
	setPageSize(); // set cr4
	enablePaging(); // set cr0
}
