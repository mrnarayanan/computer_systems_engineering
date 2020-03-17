#include "terminal.h"

#define CAPS_INDEX     0
#define L_SHIFT_INDEX  1
#define R_SHIFT_INDEX  2
#define L_CTRL_INDEX   3
#define R_CTRL_INDEX   4
#define L_ALT_INDEX    5
#define R_ALT_INDEX    6

#define VIDEO_BASE 0xB8000

uint8_t modifiers[7] = {0};             //The various modifiers that can be active
static int caps_held = 0;

//These arrays come from data on OSDev and looking at a QWERTY keyboard
const char LOWERCASE_SCANCODES[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
                                   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
                                   'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
                                   'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0};

const char SHIFTCASE_SCANCODES[] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
                                   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
                                   'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
                                   'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0};

const char CAPSLOCK_SCANCODES[] =  {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
                                   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\\', 0, 'A', 'S',
                                   'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
                                   'B', 'N', 'M', ',', '.', '/', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0};

void switch_terminal(uint8_t next_terminal_index);

/*
 * init_terminal
 *   DESCRIPTION: initializes a new terminal instance
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: modifies the passed struct
 */
void init_terminals(){
    uint8_t i, j; //counter

    for(j = 0; j < NUM_TERMINALS; j++){
        for(i = 0; i < TERMINAL_BUFFER_SIZE; i++)
            terminals[j].buffer[i] = '\0';
        terminals[j].pos_x = 0;
        terminals[j].pos_y = 0;
        terminals[j].chars_in_buffer = 0;
        terminals[j].active_process = -1;
    }
    terminals[0].video_start = (uint8_t *) VIDEO_BASE;
    terminals[1].video_start = (uint8_t *) terminal1_storage;
    terminals[2].video_start = (uint8_t *) terminal2_storage;

    terminals[0].storage_location = (uint8_t *) terminal0_storage;
    terminals[1].storage_location = (uint8_t *) terminal1_storage;
    terminals[2].storage_location = (uint8_t *) terminal2_storage;

    //Set the first terminal to be active
    active_terminal_index = 0;

    //Clear all of the vram buffers
    uint32_t k;
    for(k = VIDEO_BASE; k < terminal2_storage + KB_4; k++)
        *((uint8_t *) (VIDEO_BASE + k)) = 0;

}

/*
 * clear_terminal
 *   DESCRIPTION: writes spaces to every location in the terminal, clearing it
 *   INPUTS: terminal - a pointer to the terminal to clear
 *   OUTPUTS: a clear terminal
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Clears memory (and potentially the screen)
 */
void clear_terminal(uint8_t terminal_index){
  int32_t i;
  terminals[terminal_index].pos_x = 0;
  terminals[terminal_index].pos_y = 0;
  for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
      *(uint8_t *)(terminals[terminal_index].video_start + (i << 1)) = ' ';
      *(uint8_t *)(terminals[terminal_index].video_start + (i << 1) + 1) = ATTRIB;
  }

  if(terminal_index == active_terminal_index)
      move_cursor(0, 0);
}

/*
 * terminal_putc
 *   DESCRIPTION: writes a single character to a terminal display
 *   INPUTS:
 *     input: the character to write
 *     attribute: the attribute byte to use set for the character
 *     terminal: the terminal to write the character to
 *   OUTPUTS: modifies video memory (if terminal is active)
 *   RETRUN VALUE: none
 *   SIDE EFFECTS: writes to video memory, modifies terminal
 */
void terminal_putc(char input, uint8_t attribute, uint8_t terminal_index){
  //This code mostly comes from the old lib.c putc method, with some modifications
  if(input == '\n' || input == '\r') { //handle newline input
      terminals[terminal_index].pos_y++;
      terminals[terminal_index].pos_x = 0;
  } else {
      //write character to the memory of the associated terminal
      *(uint8_t *)(terminals[terminal_index].video_start + ((NUM_COLS * terminals[terminal_index].pos_y + terminals[terminal_index].pos_x) << 1)) = input;
      *(uint8_t *)(terminals[terminal_index].video_start + ((NUM_COLS * terminals[terminal_index].pos_y + terminals[terminal_index].pos_x) << 1) + 1) = attribute;
      terminals[terminal_index].pos_x++;

      //If horizontal overflow, reset to next line
      if(terminals[terminal_index].pos_x >= NUM_COLS){
        terminals[terminal_index].pos_y++;
        terminals[terminal_index].pos_x = 0;
      }
  }

  //Take care of scrolling screen up
  if(terminals[terminal_index].pos_y >= NUM_ROWS){
      int i; //loop counter
      for(i = 0; i < NUM_COLS * (NUM_ROWS-1); i++){
          *(uint8_t *)(terminals[terminal_index].video_start + (i << 1)) = *(uint8_t *)(terminals[terminal_index].video_start + ((i + NUM_COLS) << 1));
          *(uint8_t *)(terminals[terminal_index].video_start + (i << 1) + 1) = *(uint8_t *)(terminals[terminal_index].video_start + ((i + NUM_COLS) << 1) + 1);
      }
      //Clear last row
      for(i = i; i < NUM_COLS * NUM_ROWS; i++){
        *(uint8_t *)(terminals[terminal_index].video_start + (i << 1)) = ' ';
        *(uint8_t *)(terminals[terminal_index].video_start + (i << 1) + 1) = ATTRIB;
      }
      terminals[terminal_index].pos_y = NUM_ROWS - 1;
  }

  //At this point, update the location of the cursor
  //TODO only do this if the terminal is active
  if(terminal_index == active_terminal_index)
      move_cursor(terminals[terminal_index].pos_x, terminals[terminal_index].pos_y);
}

/*
 * terminal_read
 *   DESCRIPTION: Reads from the terminal input buffer and copies into a userspace buffer
 *   INPUTS:
 *     - uint8_t *buffer: the userspace buffer to copy into
 *     - uint32_t num_bytes: The maximum number of bytes to copy into the buffer
 *     - terminal_t *terminal: The terminal to copy from
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes copied into the buffer
 *   SIDE EFFECTS: modifes terminal input buffer and userspace buffer
 */
int32_t terminal_read(int32_t fd, char *buffer, int32_t num_bytes){
    pcb_t * pcb = getCurrentProcessPCB();
    if(fd == 1) return -1;    //Invalid read from stdout

    if(buffer == NULL) return -1;
    uint32_t flags;
    if(num_bytes > TERMINAL_BUFFER_SIZE) num_bytes = TERMINAL_BUFFER_SIZE;

    //It is possible that the number of bytes requested does not have a newline in it
    //This handles that case
    //TODO: fix possibility that a newline is pressed before read called and it isn't seen
    sti();
    while(terminals[pcb->terminal_index].chars_in_buffer < num_bytes){
        if(terminals[pcb->terminal_index].buffer[terminals[pcb->terminal_index].chars_in_buffer - 1] == '\n') //If a newline, return now
            break;
    }
    cli_and_save(flags);  //Critical section - no backspaces
    uint32_t retval = terminals[pcb->terminal_index].chars_in_buffer;
    //Check if should return fewer characters
    if(num_bytes < retval) retval = num_bytes;

    //Remove from input buffer
    terminals[pcb->terminal_index].chars_in_buffer -= retval;
    uint32_t i; //loop counter
    for(i = 0; i < retval; i++)
        buffer[i] = terminals[pcb->terminal_index].buffer[i];
    uint32_t index = retval;
    for(i = 0; index < TERMINAL_BUFFER_SIZE; index++){
        terminals[pcb->terminal_index].buffer[i++] = terminals[pcb->terminal_index].buffer[index];
    }

    restore_flags(flags); //end critical section

    return retval;
}

/*
 * terminal_write
 *   DESCRIPTION: Writes a string to a terminal until it hits a null character or reaches the number of bytes passed
 *   INPUTS:
 *     - uint8_t* buffer: the buffer holding the string
 *     - uint32_t characters: the maximum number of characters to write
 *     - terminal_t *terminal: the terminal to write the string to
 *   OUTPUTS: Writes to video memory
 *   RETURN VALUE: the number of bytes written to the terminal
 *   SIDE EFFECTS: writes to video memory, modifies terminal, pauses interrupts and restores to previous state
 */
int32_t terminal_write(int32_t fd, const char *buffer, int32_t characters){
  if(fd == 0) return -1;  //Trying to write to stdin

  if(buffer == NULL) return -1;
  uint32_t flags;
  cli_and_save(flags); //Pause interrupts so string is written all at once
  register int32_t index = 0;
  //Loop through string printing each character one at a time
  while (/*buffer[index] != '\0' &&*/ index < characters) {  //Assume user knows how many characters they want to write
      terminal_putc(buffer[index], ATTRIB, getCurrentProcessPCB()->terminal_index);
      index++;
  }
  restore_flags(flags);   //restore previous interrupt state
  return index;           //return the number of characters written (may be less than passed size)
}

/*
 * terminal_open
 *   DESCRIPTION: Does nothing but be called by syscalls
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: always returns 0
 *   SIDE EFFECTS: NONE
 */
int32_t terminal_open(){
    return 0;
}

/*
 * terminal_close
 *   DESCRIPTION: Does nothing but be called by syscalls
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: always returns -1
 *   SIDE EFFECTS: NONE
 */
int32_t terminal_close(){
    return -1;
}



/*
 * handle_keypress
 *   DESCRIPTION: translates a keyboard scancode into a ascii character, then prints
 *                  the character to the screen
 *   INPUTS: none (gets scancode from port)
 *   OUTPUTS: ascii character to screen
 *   RETURN VALUE: none
 *   SIDE EFFECTS: writes to screen
 */
void handle_keypress(void) {
    //Turns out we use scancode set 1
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    char pressed = 0;

    //Handle modifier keys
    if(scancode == 0x1D)
        modifiers[L_CTRL_INDEX]  = 1;
    if(scancode ==  0x2A)
        modifiers[L_SHIFT_INDEX] = 1;
    if(scancode == 0x36)
        modifiers[R_SHIFT_INDEX] = 1;
    if(scancode == 0x38)
        modifiers[L_ALT_INDEX]   = 1;
    if(scancode == 0x3A) {
        if(caps_held != 1){ //Handle capslock
            caps_held = 1;
            modifiers[CAPS_INDEX] = !(modifiers[CAPS_INDEX]);
        }
    }
    if(scancode == 0x9D)
        modifiers[L_CTRL_INDEX]  = 0;
    if(scancode == 0xAA)
        modifiers[L_SHIFT_INDEX] = 0;
    if(scancode == 0xB6)
        modifiers[R_SHIFT_INDEX] = 0;
    if(scancode == 0xB8)
        modifiers[L_ALT_INDEX]   = 0;
    if(scancode == 0xBA)
        caps_held = 0;
    if(scancode == 0xE0){
        //This scancode has a second scancode that needs to be read
        uint8_t next_scancode = inb(KEYBOARD_DATA_PORT);
        if (next_scancode == 0x1D)      modifiers[R_CTRL_INDEX] = 1;
        else if (next_scancode == 0x38) modifiers[R_ALT_INDEX]  = 1;
        else if (next_scancode == 0x9D) modifiers[R_CTRL_INDEX] = 0;
        else if (next_scancode == 0xB8) modifiers[R_ALT_INDEX]  = 0;
    }
    //This scancode represents backspace
    //TODO: fix when needing to virtualize terminals
    if(scancode == 0x0E){
        if(terminals[active_terminal_index].chars_in_buffer == 0) return; //Nothing in buffer, so don't backspace
        terminals[active_terminal_index].buffer[--(terminals[active_terminal_index].chars_in_buffer)] = '\0'; //remove last element from buffer
        //These next lines take care of setting the position correctly
        if(terminals[active_terminal_index].pos_x == 0){
            if(terminals[active_terminal_index].pos_y == 0) return; //If screen is clear, don't bother moving anything
            (terminals[active_terminal_index].pos_y)--;
            terminals[active_terminal_index].pos_x = NUM_COLS - 1;
        } else
            (terminals[active_terminal_index].pos_x)--;

        //Draw the empty space on the screen
        *(uint8_t *)(terminals[active_terminal_index].video_start + ((NUM_COLS * terminals[active_terminal_index].pos_y + terminals[active_terminal_index].pos_x) << 1)) = ' ';
        *(uint8_t *)(terminals[active_terminal_index].video_start + ((NUM_COLS * terminals[active_terminal_index].pos_y + terminals[active_terminal_index].pos_x) << 1) + 1) = ATTRIB;

        //Update the cursor position with the new position
        move_cursor(terminals[active_terminal_index].pos_x, terminals[active_terminal_index].pos_y);
    }

    if(scancode > 0x3F) return; //Scancode not valid for anything else
    if((modifiers[L_SHIFT_INDEX] || modifiers[R_SHIFT_INDEX]) && !modifiers[CAPS_INDEX]){
        pressed = SHIFTCASE_SCANCODES[scancode];
    }
    else if(!(modifiers[L_SHIFT_INDEX] || modifiers[R_SHIFT_INDEX]) && modifiers[CAPS_INDEX]){
        pressed = CAPSLOCK_SCANCODES[scancode];
    }
    else{
        pressed = LOWERCASE_SCANCODES[scancode];
    }

    //This section of code handles what modifiers do (alt, ctrl, shift, caps)
    if((modifiers[L_CTRL_INDEX] || modifiers[R_CTRL_INDEX]) && (pressed == 'l' || pressed == 'L')){
        clear_terminal(active_terminal_index);
        return;
    }
    if(modifiers[L_ALT_INDEX] || modifiers[R_ALT_INDEX]){
        if(scancode == 0x3B) switch_terminal(0);
        else if(scancode == 0x3C) switch_terminal(1);
        else if(scancode == 0x3D) switch_terminal(2);
    }

    if(pressed == 0) return; //Invalid scancode, do nothing

    if(modifiers[L_ALT_INDEX] || modifiers[R_ALT_INDEX] || modifiers[L_CTRL_INDEX] || modifiers[R_CTRL_INDEX])
        return;     //Do nothing if modifier keys are pressed, but possible handle later on

    //This code handles putting the keypress in the buffer if there is room for it, or ignores it otherwise
    //TODO: lookup active terminal
    if(terminals[active_terminal_index].chars_in_buffer < (TERMINAL_BUFFER_SIZE-1) ||
          (pressed == '\n' && terminals[active_terminal_index].chars_in_buffer < TERMINAL_BUFFER_SIZE))
    {
        terminal_putc(pressed, ATTRIB, active_terminal_index);
        terminals[active_terminal_index].buffer[terminals[active_terminal_index].chars_in_buffer] = pressed;
        terminals[active_terminal_index].chars_in_buffer++;
    }
}

/*
 * void switch_terminal(uint8_t next_terminal_index)
 *   DESCRIPTION: Switches active terminal from one to another
 *   INPUTS: next_terminal_index - the terminal to switch to
 *   OUTPUTS: Writes a new screen to memory
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Can schedule a new terminal for creation, if necessary; Writes to video memory
 */
void switch_terminal(uint8_t next_terminal_index){
    uint32_t i, process_id, entry, flags;

    if(next_terminal_index > 2 || next_terminal_index == active_terminal_index)
        return;    //Invalid terminal to swtich to, do nothing

    cli_and_save(flags);

    //Check if the other terminal has something running on it
    if(terminals[next_terminal_index].active_process != -1){
        //Something is already running, update active terminal and return
        //Copy memory from one terminal to another
        memcpy(terminals[active_terminal_index].storage_location, terminals[active_terminal_index].video_start, KB_4);
        memcpy((uint8_t *)VIDEO_BASE, terminals[next_terminal_index].storage_location, KB_4);
        move_cursor(terminals[next_terminal_index].pos_x, terminals[next_terminal_index].pos_y);
        terminals[active_terminal_index].video_start = terminals[active_terminal_index].storage_location;
        terminals[next_terminal_index].video_start = (uint8_t *) VIDEO_BASE;

        for(i = 0; i < NUM_MAX_PROCESSES; i++){
            if((video_mem_page_table[i][0] & 0x07) == 7){
                //Need to remap this page
                if((video_mem_page_table[i][0] & 0xFFFFF000) == (uint32_t)VIDEO_BASE){
                    entry = video_mem_page_table[i][0] & 0x00000FFF;
                    entry |= (uint32_t)terminals[active_terminal_index].storage_location;
                    video_mem_page_table[i][0] = entry;
                }
                else if((video_mem_page_table[i][0] & 0xFFFFF000) == (uint32_t)terminals[next_terminal_index].storage_location){
                    entry = video_mem_page_table[i][0] & 0x00000FFF;
                    entry |= (uint32_t)VIDEO_BASE;
                    video_mem_page_table[i][0] = entry;
                }
            }
        }
        loadPageDirectory(page_directory_array[curr_process - 1]);

        active_terminal_index = next_terminal_index;
        restore_flags(flags);
        return;
    }

    //Try to find an open process
    for(process_id = 0; process_id < NUM_MAX_PROCESSES; process_id++){
        if(active[process_id] == INACTIVE) break;
    }

    if(process_id >= NUM_MAX_PROCESSES) return; //No open processes, do nothing

    process_id++;    //1-index PID to be consistent

    //Copy memory from one terminal to another
    memcpy(terminals[active_terminal_index].storage_location, terminals[active_terminal_index].video_start, KB_4);
    memcpy((uint8_t *)VIDEO_BASE, terminals[next_terminal_index].storage_location, KB_4);
    move_cursor(terminals[next_terminal_index].pos_x, terminals[next_terminal_index].pos_y);
    terminals[active_terminal_index].video_start = terminals[active_terminal_index].storage_location;
    terminals[next_terminal_index].video_start = (uint8_t *) VIDEO_BASE;

    for(i = 0; i < NUM_MAX_PROCESSES; i++){
        if((video_mem_page_table[i][0] & 0x07) == 7){
            //Need to remap this page
            if((video_mem_page_table[i][0] & 0xFFFFF000) == (uint32_t)VIDEO_BASE){
                entry = video_mem_page_table[i][0] & 0x00000FFF;
                entry |= (uint32_t)terminals[active_terminal_index].storage_location;
                video_mem_page_table[i][0] = entry;
            }
            else if((video_mem_page_table[i][0] & 0xFFFFF000) == (uint32_t)terminals[next_terminal_index].storage_location){
                entry = video_mem_page_table[i][0] & 0x00000FFF;
                entry |= (uint32_t)VIDEO_BASE;
                video_mem_page_table[i][0] = entry;
            }
        }
    }
    loadPageDirectory(page_directory_array[curr_process - 1]);

    pcb_t * shell = getProcessPCB(process_id);
    shell->arg[0] = 0;
    shell->num_char_in_arg = 1;
    shell->parent_num = 0;
    shell->terminal_index = next_terminal_index;
    shell->parent_esp = 0;
    shell->parent_ebp = 0;
    shell->exec_ret_addr = 0;

    active[process_id - 1] = ACTIVE;

    //Write the shell to memory
    loadPageDirectory(page_directory_array[process_id - 1]);
    load_program((uint8_t *)"shell", (uint8_t *)V_PROGRAM_BASE);

    init_file_array(shell->file_array);
    shell->current_eip = 0;
    shell->current_eip |= ((uint8_t *)V_PROGRAM_BASE)[24];
    shell->current_eip |= ((uint8_t *)V_PROGRAM_BASE)[25] << 8;
    shell->current_eip |= ((uint8_t *)V_PROGRAM_BASE)[26] << 16;
    shell->current_eip |= ((uint8_t *)V_PROGRAM_BASE)[27] << 24;
    shell->current_ebp = 0;
    shell->current_esp = MB_128 + MB_4 - B_4;
    shell->is_user_mode = 1;

    loadPageDirectory(page_directory_array[curr_process - 1]);

    //Mark the terminal as having an active program on it
    terminals[next_terminal_index].active_process = process_id;
    active_terminal_index = next_terminal_index;
    restore_flags(flags);
}
