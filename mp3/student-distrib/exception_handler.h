#ifndef EXCEPTION_HANDLER_H
#define EXCEPTION_HANDLER_H

#include "types.h"

//This simple function will just halt the system
void stop(void);

// For now, just a generic handler for errors, showing a blue screen kernel panic
void handle_generic_error(void);

//These are all specific exceptions, listed in the order of appearance in the IDT
void handle_divide_by_zero(void);
void handle_debug_exception(void);
void handle_nmi(void);
void handle_breakpoint(void);
void handle_overflow(void);
void handle_bound_exception(void);
void handle_invalid_opcode(void);
void handle_fpu_missing(void);
void handle_double_fault(void);
void handle_coprocessor_segment_overrun(void);
void handle_invalid_tss(void);
void handle_segment_missing(void);
void handle_stack_exception(void);
void handle_general_protection(void);
void handle_page_fault();
void handle_fp_error(void);
void handle_alignment_check(void);
void handle_machine_check(void);

//THis function sets up the IDT with the various exception handlers
void setup_exception_handlers(void);

#endif
