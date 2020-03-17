#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "syscalls.h"
#include "lib.h"

//A function that sets up the RTC with an initial interrupt rate
void rtc_init();
extern int32_t rtc_open(int32_t fd);
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t rtc_close(int32_t fd);
int32_t set_freq(int32_t freq);

void rtc_int();

#define RTC_PORT 0x70	/* Address of RTC PORT */
#define DATA_PORT 0x71	/* Address of RTC's DATA PORT */
#endif
