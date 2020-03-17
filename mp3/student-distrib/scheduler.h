#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "types.h"
#include "lib.h"
#include "interrupt_handler.h"
#include "syscalls.h"
#include "terminal.h"
#include "paging.h"
#include "pcb.h" // already included in syscalls.h
#include "i8259.h"

void schedule(void);
void pit_init(void);

extern void scheduler_handler(void);

#endif
