#include "syscalls.h"

uint8_t active[NUM_MAX_PROCESSES] = {INACTIVE};
uint32_t curr_process = 0;

/* open
 * DESCRIPTION: system call for open
 * INPUTS: filename as character array
 * OUTPUTS: opens corresponding file/device/directory and populates file_array,
 * 					calls file's open function
 * RETURN VALUE: 0 on success, -1 on fail
 * SIDE EFFECTS: populates file array
 */
int32_t open(const uint8_t* filename)
{
  // fill dentry with dentry corresponding to filename
  dentry_t dentry;
  // check if file exists. if not, return -1
  if (read_dentry_by_name(filename, &dentry) == -1){
    return -1;
  }

  //check for file type
  // if rtc file, rtc_open
  if (dentry.filetype == 0) // devices are type 0
  {
    if (file_open(filename) == -1) // create RTC entry in file_array
        return -1;
    return file_open(filename);
  }
  // if directory file, directory_open
  else if (dentry.filetype == 1) // directories are type 1
  {
    return directory_open(filename);
  }
  // if normal file, file_open
  else if (dentry.filetype == 2) // regular files are type 2
  {
    return file_open(filename);
  }
  // if none of the files above, return -1
  else
  {
    return -1;
  }
}

/* close
 * DESCRIPTION: system call for close
 * INPUTS: file descriptor index
 * OUTPUTS: marks corresponding file descriptor in array as free,
 * 					calls file's close function
 * RETURN VALUE: 0 on success, -1 on failure
 * SIDE EFFECTS: modifies file array
 */
int32_t close(int32_t fd)
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  if(fd < 2 || fd > 7) // don't close stdin or stdout, or anything out of bounds
    return -1;
  if(file_array[fd].flags == FILE_AVAIL) // If file isn't open, don't bother closing it
    return -1;

  int32_t * fp = (int32_t *) file_array[fd].file_ops_table_ptr;
  CLS closer = (CLS) fp[CLOSE];
  return (*closer)(fd);
  }

/* read
 * DESCRIPTION: system call for read
 * INPUTS: file descriptor index, buffer array pointer, number of bytes to read
 * OUTPUTS: calls corresponding read function based on file type
 * RETURN VALUE: number of bytes read, -1 if failure
 * SIDE EFFECTS: none
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) // uint8_t* instead of void*
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  if (fd < 0 || fd > 7) // trying to read from invalid fd
    return -1;
  if(file_array[fd].flags == FILE_AVAIL) // file is not open
    return -1;

  int32_t * fp = (int32_t *) file_array[fd].file_ops_table_ptr;
  RD reader = (RD) fp[READ];
  return (*reader)(fd, buf, nbytes);
}

/* write
 * DESCRIPTION: system call for write
 * INPUTS: file descriptor index, buffer array pointer, number of bytes to write
 * OUTPUTS: calls corresponding write function based on file type
 * RETURN VALUE: number of bytes written, -1 if failure
 * SIDE EFFECTS: none
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) // uint8_t* instead of void*
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  if (fd < 0 || fd > 7) // trying to write to invalid fd
    return -1;
  if(file_array[fd].flags == FILE_AVAIL) // file is not open
    return -1;

  int32_t * fp = (int32_t *) file_array[fd].file_ops_table_ptr;
  WRT writer = (WRT) fp[WRITE];
  return (*writer)(fd, buf, nbytes);
}

/* halt
 * DESCRIPTION: system call for halt
 * INPUTS: status
 * OUTPUTS: switches to parent's page directory
 * RETURN VALUE: int 0 if successful
 * SIDE EFFECTS: changes curr_process
 */
int32_t halt(uint8_t status)
{
  cli();
  pcb_t * current_pcb = getCurrentProcessPCB();
  uint32_t parent_process = current_pcb->parent_num;
  uint8_t* exec_ret_addr = current_pcb->exec_ret_addr;
  uint32_t parent_ebp = current_pcb->parent_ebp;
  uint32_t parent_esp = current_pcb->parent_esp;
  uint32_t iter;

  // close open files in current process
  for (iter = 2; iter < NUM_MAX_OPEN_FILES; iter++)
  {
    close(iter);
  }

  //check if this is a root shell
  if(parent_process == 0){
      //Root shell, don't allow (true) exit, just restart
      uint8_t *program_image_storage_location = (uint8_t *) V_PROGRAM_BASE;
      uint32_t eip = program_image_storage_location[24];
      eip |= program_image_storage_location[25] << 8;
      eip |= program_image_storage_location[26] << 16;
      eip |= program_image_storage_location[27] << 24;

      //Start new instance of shell
      context_switch(eip, MB_128 + MB_4 - B_4,0);
  }

  //restore parent paging
  loadPageDirectory(page_directory_array[parent_process - 1]);
  terminals[current_pcb->terminal_index].active_process = parent_process;

  // set esp0 in TSS
  tss.esp0 = get_kernel_stack_bottom(parent_process);

  //decrement number of processes
  active[curr_process - 1] = INACTIVE;
  curr_process = parent_process;

  //jump to execute return
  execute_return( exec_ret_addr, parent_ebp, parent_esp, status );

  //stub return; this call never returns to the caller
  return 0;
}

/* execute
 * DESCRIPTION: system call for execute
 * INPUTS: command as character array
 * OUTPUTS: loads program into memory and sets new page directory
 * RETURN VALUE: int 0 if success, int -1 if fail
 * SIDE EFFECTS: modifies PCB
 */
int32_t execute(const uint8_t* command)
{
  int i, j;
  uint32_t process_id;
  uint8_t filename[FILENAME_LEN];
  uint32_t eip, flags;
  uint8_t* program_image_storage_location = (uint8_t *) V_PROGRAM_BASE; // location to load program
  uint32_t bottom_addr_of_page;
  int32_t status;
  pcb_t * child_pcb;

  cli_and_save(flags);
  //check which program pages are not being used in the kernel
  //after this loop, i contains lowest program page that is available
  for (process_id = 0; process_id < NUM_MAX_PROCESSES; process_id++){
    if (active[process_id] == INACTIVE){
      break;
    }
  }

  //maximum number of processes ongoing; return -1
  if (process_id == NUM_MAX_PROCESSES){
    return -1;
  }

  //Process ID is going to be 1-based
  process_id++;

  child_pcb = getProcessPCB(process_id);

  //get filename from command
  for (i = 0; i < FILENAME_LEN; i++){
    //if space character, break
    if (command[i] == ' '){
      //postincrementing in command[i] causes warning so I moved it out
      filename[i] = 0;
      i++;
      break;
    }
    filename[i] = command[i];
    if(command[i] == 0){
      break;
    }
  }

  while(command[i] == ' ')
    i++;  //skip sequential spaces
  for(j = 0; i < TERMINAL_BUFFER_SIZE; i++){
    child_pcb->arg[j++] = command[i];
    if(command[i] == '\n'){
      // Don't store newline as part of argument
      child_pcb->arg[j-1] = 0;
      child_pcb->num_char_in_arg = j;
      break;
    }
    if(command[i] == 0){
      child_pcb->num_char_in_arg = j;
      break;
    }
  }

  //set up program paging and increment number of processes
  //Also, set which 4MB program pages are being used
  //Since page directories are 0-based, subtract one from process id
  loadPageDirectory(page_directory_array[process_id - 1]);

  // load program into memory, return -1 if fails
  if (load_program(filename, program_image_storage_location) == -1){
      loadPageDirectory(page_directory_array[curr_process - 1]);
      return -1;
  }

  active[process_id - 1] = ACTIVE;

  //save esp first in TSS
  tss.esp0 = get_kernel_stack_bottom(process_id);

  //create PCB
  // this should set files for stdin and stdout in file array
  init_file_array(child_pcb->file_array);

  //copy entry point to eip - bytes 24-27
  eip = 0;
  eip |= program_image_storage_location[24];
  eip |= program_image_storage_location[25] << 8;
  eip |= program_image_storage_location[26] << 16;
  eip |= program_image_storage_location[27] << 24;

  //store parent process number in PCB
  child_pcb->parent_num = curr_process;

  //store execute return address in PCB
  child_pcb->exec_ret_addr = &&end_of_execute; //__builtin_return_address(0);
  child_pcb->terminal_index = getCurrentProcessPCB()->terminal_index;
  terminals[child_pcb->terminal_index].active_process = process_id;

  //save esp and ebp in PCB
  asm("\t movl %%esp,%0" : "=r"(child_pcb->parent_esp));
  asm("\t movl %%ebp,%0" : "=r"(child_pcb->parent_ebp));

  //set current process as the process being switched into
  curr_process = process_id;

  //perform the context switch
  bottom_addr_of_page = MB_128 + MB_4 - B_4;
  context_switch(eip, bottom_addr_of_page,0);

  end_of_execute:;
  //move eax into status
  asm("\t movl %%eax,%0" : "=r" (status));
  restore_flags(flags);

  //return the status passed by halt
  return status;
}

/* getargs
 * DESCRIPTION: system call for getargs
 * INPUTS: pointer to buffer, number of bytes
 * OUTPUTS: reads from PCB into buffer
 * RETURN VALUE: 0 on success, -1 on fail
 * SIDE EFFECTS: reads from PCB
 */
int32_t getargs(uint8_t* buf, int32_t nbytes)
{
  //if location is invalid, return -1
  //Check if address of screen_start is within the address range covered
  //by the single user-level page for the process.
  //0x8000000 is 128MB, 0x400000 is 4MB
  //I'm making assumptions about process mapping from virtual to physical
  //Specifically, process 0 is mapped to 128-132MB page, process 1 to 132-136MB page
  if (buf == NULL ||
     (uint32_t) buf > MB_128 + MB_4  ||
     (uint32_t) buf < MB_128){
    return -1;
  }

  pcb_t * current_pcb = getCurrentProcessPCB();

  if(current_pcb->num_char_in_arg == 1) // no arguments passed
    return -1;
  if(nbytes < current_pcb->num_char_in_arg) // can't fit arg in buffer
    return -1;

  uint32_t i;
  for(i = 0; i < current_pcb->num_char_in_arg; i++)
    buf[i] = current_pcb->arg[i];

  return 0;
}

/* vidmap
 * DESCRIPTION: system call for vidmap
 * INPUTS: double pointer to screen_start
 * OUTPUTS: sets pointer for user to be able to modify video memory
 * RETURN VALUE: int 0 for success, int -1 if fail
 * SIDE EFFECTS: modifies page directory
 */
int32_t vidmap(uint8_t** screen_start)
{
  int i;
  uint32_t flags;
  cli_and_save(flags);
  //if location is invalid, return -1
  //Check if address of screen_start is within the address range covered
  //by the single user-level page for the process.
  //0x8000000 is 128MB, 0x400000 is 4MB
  //I'm making assumptions about process mapping from virtual to physical
  //Specifically, process 0 is mapped to 128-132MB page, process 1 to 132-136MB page
  if (screen_start == NULL ||
     (uint32_t) screen_start > MB_128 + MB_4 - B_4 ||
     (uint32_t) screen_start < MB_128){
    return -1;
  }

  // map text-mode video memory into user space at virtual 256MB
  *screen_start = (uint8_t*) MB_256; // 256 MB

  //set paging for that virtual address
  //initialize first page table containing video memory (video memory at location 0xB8000)
  video_mem_page_table[curr_process - 1][0] = (184 * KB_4) | 7;
  for(i = 1; i < NUM_ENTRIES; i++)
  {
    // As the address is page aligned, it will always leave 12 bits zeroed.
    // attributes: supervisor level, read/write, present.
    //set video memory page table to point to addresses starting at 0MB
    //184 because video memory is mapped to physical 0xB8000
    //
    video_mem_page_table[curr_process - 1][i] = (i * KB_4) | 6;
  }

  //Map from virtual 256MB to 0MB
  page_directory_array[curr_process - 1][64] = ((unsigned int) video_mem_page_table[curr_process - 1]) | 7;

  //Flush TLB
  loadPageDirectory(page_directory_array[curr_process - 1]);
  restore_flags(flags);

  return 0;
}

/* set_handler
 * DESCRIPTION: system call for set_handler
 * INPUTS:
 * OUTPUTS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 */
int32_t set_handler(int32_t signum, void* handler) // uint8_t* instead of void*
{

  return -1;
}

/* sigreturn
 * DESCRIPTION: system call for sigreturn
 * INPUTS:
 * OUTPUTS:
 * RETURN VALUE:
 * SIDE EFFECTS:
 */
int32_t sigreturn(void)
{

  return -1;
}

/*
 * pcb_t * getCurrentProcessPCB()
 *   DESCRIPTION: returns pointer to the current PCB
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: A pointer to the active PCB
 *   SIDE EFFECTS: none
 */
pcb_t * getCurrentProcessPCB(){
  uint32_t esp;
  asm volatile ("\t movl %%esp,%0" : "=r"(esp));
  return (pcb_t *)(esp & ~(PCB_MASK));
}

pcb_t * getProcessPCB(uint32_t pid){
  return (pcb_t *)(P_PROCESS_BASE - (pid + 1) * P_PROCESS_OFFSET);
}

uint32_t get_kernel_stack_bottom(uint32_t pid){
  return P_PROCESS_BASE - pid * P_PROCESS_OFFSET - B_4;
}
