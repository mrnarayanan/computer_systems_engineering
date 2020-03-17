/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_interrupt_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_interrupt_mask = 0xFF;  /* IRQs 8-15 */

/* void i8259_init(void)
 * 		DESCRIPTION: Initializes the master and slave 8259 PIC chips
 * 		INPUTS: none
 * 		OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Initializes master and slave PICs
 */
void i8259_init(void) {

	/*Masks interrupts into the PIC by writing 0xFF to the data ports of both Master and Slave which is +1 from the command ports of both Master and Slave ports*/
	outb(master_interrupt_mask, MASTER_8259_PORT + 1);
	outb(slave_interrupt_mask, SLAVE_8259_PORT + 1);

	/*Sends the ICWs to their respective PICs in the order of ICW1, 2, 3 and lastly 4 to initialize PIC*/
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);

	outb(ICW2_MASTER, MASTER_8259_PORT + 1);
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);

	outb(ICW3_MASTER, MASTER_8259_PORT + 1);
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);

	outb(ICW4, MASTER_8259_PORT + 1);
	outb(ICW4, SLAVE_8259_PORT + 1);
}


/*
 * void enable_irq(uint32_t irq_num)
 * 		DESCRIPTION: Enable (unmask) the specified IRQ
 * 		INPUTS: irq_num - the IRQ number to unmask
 * 		OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Unmasks the IRQ line specified by irq_num no the external PIC
 */
void enable_irq(uint32_t irq_num) {
	if(irq_num > NUMBER_IRQS) return;  // If irq_num > 15, it is invalid

	if(irq_num < 8){	/* Checks if IRQ number is less than 8, which means its an IRQ for the Master PIC */
		master_interrupt_mask = master_interrupt_mask & ~(1 << irq_num); /*Unmasks the corresponding IRQ number */
		outb(master_interrupt_mask, MASTER_8259_PORT + 1);
	}
	else{				/* If its 8 or higher, it belongs in the Slave PIC */
		slave_interrupt_mask = slave_interrupt_mask & ~(1 << (irq_num - 8)); /*Unmasks the corresponding IRQ number, 8 corresponds to the number of IRQ pins on Master */
		outb(slave_interrupt_mask, SLAVE_8259_PORT + 1);
		master_interrupt_mask = master_interrupt_mask & ~(1 << ICW3_SLAVE); //Unmask master port that slave is on
		outb(master_interrupt_mask, MASTER_8259_PORT + 1);
	}
}

/*
 * void disable_irq(uint32_t irq_num)
 * 		DESCRIPTION: Disable (mask) the specific IRQ
 * 		Input: irq_num - the IRQ number to mask
 * 		OUTPUT: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: masks the interrupt on the PIC so it isn't thrown
 */
void disable_irq(uint32_t irq_num) {
	if(irq_num > NUMBER_IRQS) return;  // If irq_num > 15, it is invalid

	if(irq_num < 8){	/* Checks if IRQ number is less than 8, which means its an IRQ for the Master PIC as Master PIC only governs pins 0 to 7 */
		master_interrupt_mask = master_interrupt_mask | (1 << irq_num); /*Masks the corresponding IRQ number */
		outb(master_interrupt_mask, MASTER_8259_PORT + 1);
	}
	else{				/* If its 8 or higher, it belongs in the Slave PIC */
		slave_interrupt_mask = slave_interrupt_mask | (1 << (irq_num - 8)); /*Unmasks the corresponding IRQ number*/
		outb(slave_interrupt_mask, SLAVE_8259_PORT + 1);
		if(slave_interrupt_mask == 0xFF){           //If slave is completly disabled, disable master port
			master_interrupt_mask = master_interrupt_mask | (1 << ICW3_SLAVE);
			outb(master_interrupt_mask, MASTER_8259_PORT + 1);
		}
	}
}

/*
 * send_eoi
 *   DESCRIPTION: Sends an End-of-Interrupt signal to the IRQ line specified by irq_num
 *   INPUTS: irq_num: The number of the IRQ line to send the EOI to
 *   OUTPUTS: Sends an EOI signal
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sends EOI
 */
void send_eoi(uint32_t irq_num) {
	if(irq_num > NUMBER_IRQS) return;  // If irq_num > 15, it is invalid

	if(irq_num < 8){	/* Checks if IRQ number is less than 8, which means its an IRQ for the Master PIC as Master PIC only governs pins 0 to 7 */
		outb((EOI | irq_num), MASTER_8259_PORT);	/* Adds the IRQ number to EOI and sends it to the master PIC port */
	}
	else{				/* If its 8 or higher, it belongs in the Slave PIC */
		outb((EOI | (irq_num - 8)), SLAVE_8259_PORT); /*Subtracts irq_num by 8, the number of pins on Master, then adds it to EOI, then sends it to port */
		outb((EOI | 2), MASTER_8259_PORT); /*Also sends the corresponding command to Master port to tell it that there was an end of interrupt to slave */
	}
}
