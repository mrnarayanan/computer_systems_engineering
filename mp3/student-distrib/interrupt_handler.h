#ifndef INTERRUPT_HANDLER_H
#define INTERRUPT_HANDLER_H

#define RTC_PORT 0x70
#define RTC_DATA_PORT 0x71
#define KEYBOARD_DATA_PORT 0x60

#ifndef ASM

/* This method acts as an assembly wrapper for the RTC interrupt, and acts to save all the registers before calling a c handler */
extern void rtc_handler(void);
/* This method acts as an assembly wrapper for a keypress, saves all registers before calling a c handler */
extern void key_handler(void);
/* This method acts as an assembly wrapper for all system trap calls */
extern void trap_handler(void);
/* This method acts as an assembly wrapper for scheduler */
extern void scheduler_handler(void);

extern void system_call_handler(void);

extern int32_t do_halt (uint8_t status);
extern int32_t do_execute (const uint8_t* command);
extern int32_t do_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t do_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t do_open (const uint8_t* filename);
extern int32_t do_close (int32_t fd);
extern int32_t do_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t do_vidmap (uint8_t** screen_start);
extern int32_t do_set_handler (int32_t signum, void* handler);
extern int32_t do_sigreturn (void);

#endif //asm

#endif //int handler
