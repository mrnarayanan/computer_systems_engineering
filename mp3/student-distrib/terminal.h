#ifndef TERMINAL_H
#define TERMINAL_H

#define TERMINAL_BUFFER_SIZE 128

#include "types.h"
#include "lib.h"
#include "interrupt_handler.h"
#include "syscalls.h"

#define NUM_TERMINALS 3

//These are the out-of-video-memory storage addresses for terminals
#define terminal0_storage 0xB9000
#define terminal1_storage 0xBA000
#define terminal2_storage 0xBB000

//This is the curerntly active terminal index
uint8_t active_terminal_index;

extern const char LOWERCASE_SCANCODES[];
extern const char SHIFTCASE_SCANCODES[];
extern const char CAPSLOCK_SCANCODES[];

/*
 * A lot of the functions in this file take a terminal_t pointer.
 * That is done in order to make it easier later on to virtualize the terminal -
 *   there can be multiple terminals running at once, but only one should get keyboard
 *   input and have control to write to the video memory.
 * This struct should hopefully make that easier.
 */
typedef struct terminal_t{
    char buffer[TERMINAL_BUFFER_SIZE];
    uint8_t* video_start;
    uint8_t* storage_location;
    uint16_t pos_x;
    uint16_t pos_y;
    uint16_t chars_in_buffer;
    int8_t   active_process;
} terminal_t;

terminal_t terminals[NUM_TERMINALS];

/* This is the method that handles when a key is pressed on the keyboard as an interrupt */
void handle_keypress();

/* This method initalizes a terminal */
void init_terminals();

/* This method writes a string to a terminal */
int32_t terminal_write(int32_t fd, const char *buffer, int32_t characters);

/* This method reads from the input buffer to a userspace buffer */
int32_t terminal_read(int32_t fd, char *buffer, int32_t num_bytes);

/* These methods are called by systemcalls, but they do nothing really */
int32_t terminal_open();
int32_t terminal_close();

/* This method writes a single character to a terminal */
void terminal_putc(char input, uint8_t attribute, uint8_t terminal_index);

/* This method will zero out a terminal, clearing it */
void clear_terminal(uint8_t terminal_index);


#endif
