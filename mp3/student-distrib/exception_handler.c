#include "exception_handler.h"
#include "lib.h"
#include "x86_desc.h"
#include "interrupt_handler.h"

/*
 * stop
 *   DESCRIPTION: Stops the system
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: turn off interrupts and won't return to calling function
 */
void stop(void){
    cli();                                //Turn off interrupts
    asm volatile (".2: hlt; jmp .2;");    //Just spin repeatedly as a halt
}


/*
 * handle_generic_error
 *   DESCRIPTION: Handle any error that is not handled by another handler
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen kernel panic message and halt the system
 */
void handle_generic_error(void){
    blue_screen();
    printf("Kernel Panic:\nUndefined Exception");
    stop();
}

/*
 * setup_exception_handlers
 *   DESCRIPTION: Set up the IDT with the various pointers to functions that handle exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN_VALUE: none
 *   SIDE EFFECTS: sets up the first 32 entries of the IDT
 */
void setup_exception_handlers(void){
    SET_IDT_ENTRY(idt[0],  &handle_divide_by_zero);          //IDT 00
    SET_IDT_ENTRY(idt[1],  &handle_debug_exception);         //IDT 01
    SET_IDT_ENTRY(idt[2],  &handle_nmi);                     //IDT 02
    SET_IDT_ENTRY(idt[3],  &handle_breakpoint);              //IDT 03
    SET_IDT_ENTRY(idt[4],  &handle_overflow);                //IDT 04
    SET_IDT_ENTRY(idt[5],  &handle_bound_exception);         //IDT 05
    SET_IDT_ENTRY(idt[6],  &handle_invalid_opcode);          //IDT 06
    SET_IDT_ENTRY(idt[7],  &handle_fpu_missing);             //IDT 07
    SET_IDT_ENTRY(idt[8],  &handle_double_fault);            //IDT 08
    SET_IDT_ENTRY(idt[9],  &handle_coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[10], &handle_invalid_tss);             //IDT 10
    SET_IDT_ENTRY(idt[11], &handle_segment_missing);         //IDT 11
    SET_IDT_ENTRY(idt[12], &handle_stack_exception);         //IDT 12
    SET_IDT_ENTRY(idt[13], &handle_general_protection);      //IDT 13
    SET_IDT_ENTRY(idt[14], &handle_page_fault);              //IDT 14
    SET_IDT_ENTRY(idt[15], &handle_generic_error);           //IDT 15: Reserved
    SET_IDT_ENTRY(idt[16], &handle_fp_error);                //IDT 16
    SET_IDT_ENTRY(idt[17], &handle_alignment_check);         //IDT 17
    SET_IDT_ENTRY(idt[18], &handle_machine_check);           //IDT 18

    //The rest of the exceptions (19-31) are reserved. Use generic handler
    uint32_t i;
    for(i = 19; i < 0x20; i++)
      SET_IDT_ENTRY(idt[i], &handle_generic_error);

    //For now, set up rest of handlers with generic exception
    for( ; i < NUM_VEC; i++) {
      SET_IDT_ENTRY(idt[i], &handle_generic_error);
    }
}

/*
 * handle_divide_by_zero
 *   DESCRIPTION: Handle divide by 0 exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_divide_by_zero(void){
    blue_screen();
    printf("Kernel Panic:\nDivision by 0");
    stop();
}

/*
 * handle_debug_exception
 *   DESCRIPTION: Handle debug exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_debug_exception(void){
    blue_screen();
    printf("Kernel Panic:\nDebug Exception");
    stop();
}

/*
 * handle_nmi
 *   DESCRIPTION: Handle non maskable interrupt
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_nmi(void){
    blue_screen();
    printf("Kernel Panic:\nNon-Maskable Interrupt");
    stop();
}

/*
 * handle_breakpoint
 *   DESCRIPTION: Handle breakpoints (INT3)
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_breakpoint(void){
    blue_screen();
    printf("Kernel Panic:\nBreakpoint (INT3)");
    stop();
}

/*
 * handle_overflow
 *   DESCRIPTION: Handle overflow exceptions when a flag is set
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_overflow(void){
    blue_screen();
    printf("Kernel Panic:\nOverflow with EFLAGS[OF] Set");
    stop();
}

/*
 * handle_bound_exception
 *   DESCRIPTION: Handle bound exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_bound_exception(void){
    blue_screen();
    printf("Kernel Panic:\nDebug Exception");
    stop();
}

/*
 * handle_invalid_opcode
  *   DESCRIPTION: Handle debug exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_invalid_opcode(void){
    blue_screen();
    printf("Kernel Panic:\nInvalid Opcode");
    stop();
}

/*
 * handle_fpu_missing
 *   DESCRIPTION: Handle floating point unit not available
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_fpu_missing(void){
    blue_screen();
    printf("Kernel Panic:\nFloating Point Unit Missing");
    stop();
}

/*
 * handle_double_fault
 *   DESCRIPTION: Handle double fault
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_double_fault(void){
    blue_screen();
    printf("Kernel Panic:\nDouble Fault");
    stop();
}

/*
 * handle_coprocessor_segment_overrun
 *   DESCRIPTION: Handle coprocessor segment overruns
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_coprocessor_segment_overrun(void){
    blue_screen();
    printf("Kernel Panic:\nCoprocessor Segment Overrun");
    stop();
}

/*
 * handle_invalid_tss
 *   DESCRIPTION: Handle invalid TSS exception
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_invalid_tss(void){
    blue_screen();
    printf("Kernel Panic:\nInvalid TSS");
    stop();
}

/*
 * handle_segment_missing
 *   DESCRIPTION: Handle segment not present exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_segment_missing(void){
    blue_screen();
    printf("Kernel Panic:\nSegment Not Present");
    stop();
}

/*
 * handle_stack_exception
 *   DESCRIPTION: Handle stack exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_stack_exception(void){
    blue_screen();
    printf("Kernel Panic:\nStack Exception");
    stop();
}

/*
 * handle_general_protection
 *   DESCRIPTION: Handle general protection exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_general_protection(void){
    blue_screen();
    printf("Kernel Panic:\nGeneral Protection Exception");
    stop();
}

/*
 * handle_page_fault
 *   DESCRIPTION: Handle page fault exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_page_fault(void){
    blue_screen();
    printf("Kernel Panic:\nPage Fault\nException Code: %x", __builtin_return_address(0));
    stop();
}

/*
 * handle_fp_error
 *   DESCRIPTION: Handle floating point error exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_fp_error(void){
    blue_screen();
    printf("Kernel Panic:\nFloating Point Error");
    stop();
}

/*
 * handle_alignment_check
 *   DESCRIPTION: Handle alignment check exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_alignment_check(void){
    blue_screen();
    printf("Kernel Panic:\nAlignment Check");
    stop();
}

/*
 * handle_machine_check
 *   DESCRIPTION: Handle machine check exceptions
 *   INPUTS: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print a blue screen error message and halt the system
 */
void handle_machine_check(void){
    blue_screen();
    printf("Kernel Panic:\nMachine Check Exception");
    stop();
}
