#include "rtc.h"

#define RTC_1024_HZ 0x06;

/*
 *	Code is based on the code written in https://wiki.osdev.org/RTC
 */

/*
 * void rtc_init()
 *   DESCRIPTION: initializes RTC
 * 	 INPUT: None
 * 	 OUTPUT: enables periodic RTC interrupts and sets RTC interrupt rate to 1024 Hz
 *   SIDE EFFECTS: Modifies RTC registers
 *   RETURN VALUE: none
 */
void rtc_init(){
	uint32_t flags, rate;
	cli_and_save(flags);

	outb(0x8B, RTC_PORT);			/* select register B, and disable non maskable interrupts to prevent any interruptions to rtc_init */
	char prev = inb(DATA_PORT);		/* read the data stored in register B */
	outb(0x8B, RTC_PORT);			/* set the index again because the read will reset the value in register D */
	outb((prev | 0x40), DATA_PORT);	/* write the previous value ORed with 0x40 which will turn on bit number 6 of register B */

	//This code is taken from OSDEV
	 outb(0x8A, RTC_PORT);			/* select register B, and disable NMI to the RTC port */
	 prev=inb(DATA_PORT);		/* read the current value of register B */
	 rate = (prev & 0xF0) | RTC_1024_HZ ; 	/* Updates the bottom 4 bits but conserves the top 4 bits of temp; */
	 outb(0x8A, RTC_PORT);			/* set the index again */
	 outb(rate, DATA_PORT);			/* writes the value of the new frequency into RTC */

	restore_flags(flags);
}

/*
 * int32_t rtc_open(int32_t fd)
 *   DESCRIPTION: Opens virtual RTC, sets it to 2Hz as a default user interrupt rate
 * 	 INTPUT: int32_t fd: The file descriptor array index that this RTC instance is being assigned to
 * 	 OUTPUT: none
 *   SIDE EFFECTS: will modify PCB and file descriptor array
 *   RETURN VALUE: always returns 0
 */
int32_t rtc_open(int32_t fd){
		pcb_t * pcb = getCurrentProcessPCB();
		pcb->file_array[fd].file_position = 2;    //Default interrupt rate is 2 hertz
		pcb->file_array[fd].flags = FILE_OCCUP;

		return 0;
}

/*
 * int32_t rtc_close(int32_t fd)
 *   DESCRIPTION: Closes RTC and resets the FD
 * 	 INTPUT: int32_t fd: The filedescriptor to close
 * 	 OUTPUT: none
 *   SIDE EFFECTS: Modifies PCB/file descriptor and sets a file as closed
 *   RETURN VALUE: -1 if file isn't open, 0 if successful
 */
int32_t rtc_close(int32_t fd){
	pcb_t * pcb = getCurrentProcessPCB();

	if(pcb->file_array[fd].flags == FILE_AVAIL)
			return -1;

	pcb->file_array[fd].file_position = 0;
	pcb->file_array[fd].file_ops_table_ptr = 0;
	pcb->file_array[fd].inode_num = 0;
	pcb->file_array[fd].flags = FILE_AVAIL;

	return 0;
}

/*
 * int32_t rtc_read(void* buf, int32_t nbytes)
 * 		DESCRIPTION: Reads data from the RTC. With virtualization, it would take the virtual rtc freq from the PCB, calculate the number of interrupts before returning from it.
 *					 With virtualization, the interrupt flag is incremented instead of only having 0 or 1 as possible values.
 * 		INTPUT: None
 * 		OUTPUT: Returns 0
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
	pcb_t* pcb = getCurrentProcessPCB();
	pcb->rtc_count = 1024 / (pcb->file_array[fd].file_position);
	while(pcb->rtc_count != 0)	/* Waits until interrupt counter % calculated value == 0, then returns 0*/
	{
	}
	return 0;
}

/*
 * int32_t rtc_write(const void* buf, int32_t nbytes)
 * 		DESCRIPTION: Checks if the inputs are valid or not and writes it to the rtc frequency field of the PCB.
 * 		INTPUT: Input buffer of the frequency and number of bytes to be written
 * 		OUTPUT: Number of bytes written
 *    RETURN VALUE: The number of bytes written, or -1 if an invalid amount
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
	if(buf == NULL || nbytes != 4) /*Checks if buffer is NULL or if the number of bytes is not equal to 4(frequency can only be a 4 byte integer). If either are true, return -1 */
		{
			return -1;
		}
	/*If valid, writes the frequency into the current pcb_t's frequency field */
	int freq;
	pcb_t* pcb = getCurrentProcessPCB();
	freq = *(int32_t*) buf;
	if(freq < 2 || freq > 1024 || (freq & (freq - 1)))  //Check if valid requested frequency
			return -1;

	pcb->file_array[fd].file_position = freq;
	return 4;
}

/*
 * void rtc_int()
 *   DESCRIPTION: Called when an RTC interrupt occurs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Allows for an RTC interrupt to return, modifies PCB by changing rtc_count
 */
void rtc_int(){
		uint8_t i;
		uint32_t flags;
		cli_and_save(flags);
		for(i = 0; i < NUM_MAX_PROCESSES; i++){
				pcb_t * pcb = getProcessPCB(i + 1);   //Iterate through all PCBs to update the RTC count
				if(pcb->rtc_count != 0) (pcb->rtc_count)--;
		}

		//This code reads from the rtc to allow next interrupt
		outb(0x0C, 0x70);
		inb(0x71);
		restore_flags(flags);
}
