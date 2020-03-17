#include "vfs.h"

/* code for virtual file system driver */
/* functions based off of discussion slides */
/* function headers based off of ece391syscall.h */

static uint8_t * fs_ptr;
static file_t blank;
static int32_t terminal_ops[4] = { (int32_t) &terminal_open, (int32_t) &terminal_read, (int32_t) &terminal_write, (int32_t) &terminal_close}; // open, read, write, close
static int32_t file_ops[4] = { (int32_t) &file_open, (int32_t) &file_read, (int32_t) &file_write, (int32_t) &file_close}; // open, read, write, close
static int32_t directory_ops[4] = { (int32_t) &directory_open, (int32_t) &directory_read, (int32_t) &directory_write, (int32_t) &directory_close}; // open, read, write, close
static int32_t rtc_ops[4] = { (int32_t) &rtc_open, (int32_t) &rtc_read, (int32_t) &rtc_write, (int32_t) &rtc_close}; // open, read, write, close


/* load_program
 * DESCRIPTION: loads program file into memory
 * INPUTS: name of executable file, pointer to where to copy program in memory
 * OUTPUTS: copies program file contents to memory
 * RETURN VALUE: 0 if success, -1 if fail
 * SIDE EFFECTS: none
 */
int32_t load_program(const uint8_t* filename, uint8_t * ptr)
{
  dentry_t new_dent;
  uint32_t count = 0;
  while(filename[count] != NULL)
  {
    count++;
  }
  //off by 1 possibly; take a look later
  if (count >= 33)
    return -1;
  //file with matching file name not found, return -1
  if (read_dentry_by_name(filename, &new_dent) == -1)
    return -1;

  if(new_dent.filetype != 2)
    return -1; //Not a normal file to open

  uint32_t inode = new_dent.inode_num;
  uint32_t offset = 0;
  inode_block_t* inode_block = (inode_block_t*) (fs_ptr + NUM_B_IN_FOUR_KB * (inode + 1));
  uint32_t size = inode_block->length;
//  int8_t buf[size];

  if (read_data(inode, offset, ptr, size) != size) // fail if entire executable file was not read
    return -1;

  uint8_t first = ptr[0];
  uint8_t second = ptr[1];
  uint8_t third = ptr[2];
  uint8_t fourth = ptr[3];

  // if file isn't an executable
  if ( first != 0x7F || second != 0x45 || third != 0x4C || fourth != 0x46 ) // magic numbers to specify executable file
    return -1;

  return 0;
}


/* file_open
 * DESCRIPTION: initialize any temporary structures
 * INPUTS: pointer to 8 bit filename
 * OUTPUTS:
 * RETURN VALUE: int fd on success, int -1 on failure
 * SIDE EFFECTS: uses read_dentry_by_name
 */
int32_t file_open(const uint8_t* filename)
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  dentry_t new_dent;
  int32_t index;
  uint32_t count = 0;

  if (filename == NULL){
    return -1;
  }

  while(filename[count] != NULL)
  {
    count++;
  }
  if (count >= 33)
    return -1;
  //file with matching file name not found, return -1
  if (read_dentry_by_name(filename, &new_dent) == -1)
    return -1;

  // find next available opening in file_array. skip 0 and 1 because taken by stdin and stdout
  for (index = 2; index < NUM_MAX_OPEN_FILES; index++) // start at 2 b/c first two are reserved
  {
    if (file_array[index].flags == FILE_AVAIL) // using flag value to determine if free space in file_array
      break;
  }

  //check if file array is already filled
  if (index >= NUM_MAX_OPEN_FILES)
    return -1; // fail b/c no free space in file array

  if (new_dent.filetype == 2) // regular file
  {
    file_array[index].file_ops_table_ptr = (int32_t) file_ops;
    file_array[index].inode_num = new_dent.inode_num;
  }
  else if (new_dent.filetype == 1) // directory
  {
    file_array[index].file_ops_table_ptr = (int32_t) directory_ops;
    file_array[index].inode_num = new_dent.inode_num;
  }
  else if (new_dent.filetype == 0) // RTC
  {
    file_array[index].file_ops_table_ptr = (int32_t) rtc_ops;
    file_array[index].inode_num = new_dent.inode_num;
    rtc_open(index);
    return index;
  }
  else // unknown file type, return -1 for failure
  {
    return -1;
  }

  // certain values same for all files when init
  file_array[index].file_position = 0;
  file_array[index].flags = FILE_OCCUP;

  return index;
}

/* file_close
 * DESCRIPTION: undoes initialization done in file_open
 * INPUTS: file descriptor (32 bit)
 * OUTPUTS:
 * RETURN VALUE: int 0 on success, int -1 on failure
 * SIDE EFFECTS:
 */
int32_t file_close(int32_t fd)
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  if (fd >= NUM_MAX_OPEN_FILES || fd < 0) // index out of bounds
    return -1;
  else if (file_array[fd].flags == FILE_AVAIL) // erroneously trying to close an unopened file.
    return -1;
  else
    file_array[fd] = blank;
  return 0;
}

/* file_read
 * DESCRIPTION: reads count bytes of data from file into buf
 * INPUTS: file descriptor, pointer to buffer, number of bytes
 * OUTPUTS:
 * RETURN VALUE: number of bytes read
 * SIDE EFFECTS: uses read_data
 */
int32_t file_read(int32_t fd, int8_t* buf, int32_t nbytes)
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  if (fd == 1 || fd == 0) // don't want to read from stdout or stdin using file_read
    return -1;

  uint32_t inode = file_array[fd].inode_num;
  uint32_t offset = file_array[fd].file_position;

  //return number of bytes read, update file position
  uint32_t read = read_data(inode, offset, (uint8_t *) buf, (uint32_t) nbytes);
  file_array[fd].file_position += read;
  return read;
}

/* file_write
 * DESCRIPTION: does nothing b/c this is a read only file system
 * INPUTS: file descriptor, pointer to buffer, number of bytes
 * OUTPUTS: always return -1
 * RETURN VALUE: int 0 on success, int -1 on failure
 * SIDE EFFECTS:
 */
int32_t file_write(int32_t fd, const int8_t* buf, int32_t nbytes)
{
  return -1; // this function does nothing, always fails
}

/* directory_open
 * DESCRIPTION: opens a directory file
 * INPUTS: pointer to 8 bit filename
 * OUTPUTS:
 * RETURN VALUE: int fd on success, int -1 on failure
 * SIDE EFFECTS: uses read_dentry_by_name
 */
int32_t directory_open(const uint8_t* filename)
{
  dentry_t new_dent;

  //file with matching file name not found, return -1
  if (read_dentry_by_name(filename, &new_dent) == -1)
      return -1;
  if (new_dent.filetype != 1) // not a directory
    return -1;

  return file_open(filename);
}

/* directory_close
 * DESCRIPTION: probably does nothing
 * INPUTS: file descriptor (32 bit)
 * OUTPUTS:
 * RETURN VALUE: int 0 on success, int -1 on failure
 * SIDE EFFECTS:
 */
int32_t directory_close(int32_t fd)
{
  return file_close(fd);
}

/* directory_read
 * DESCRIPTION: reads files filename by filename, including "."
 * INPUTS: file descriptor, pointer to buffer, number of bytes
 * OUTPUTS:
 * RETURN VALUE: number of bytes read
 * SIDE EFFECTS: uses read_dentry_by_index
 */
int32_t directory_read(int32_t fd, int8_t* buf, int32_t nbytes)
{
  file_t * file_array = getCurrentProcessPCB()->file_array;
  dentry_t new_dent;
  int32_t i = 0;
  if (read_dentry_by_index(file_array[fd].file_position, &new_dent) == -1)
    return -1;
  file_array[fd].file_position++;
  while (new_dent.filename[i] != '\0' && i < 32) {
      buf[i] = new_dent.filename[i];
      i++;
  }
  buf[i] = '\0';
  return i;
}

/* directory_write
 * DESCRIPTION: does nothing b/c this is a read only file system
 * INPUTS: file descriptor, pointer to buffer, number of bytes
 * OUTPUTS: always return -1
 * RETURN VALUE: int 0 on success, int -1 on failure
 * SIDE EFFECTS:
 */
int32_t directory_write(int32_t fd, const int8_t* buf, int32_t nbytes)
{
  return -1; // this function does nothing, always fails
}

// function interfaces copied from lecture slides
/* read_dentry_by_name
 * DESCRIPTION: reads directory entry by name
 * INPUTS: fname - filename
 * OUTPUTS: corresponding directory entry stored in dentry
 * RETURN VALUE: int 0 on success, int -1 on failure
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
  //Scans through the directory entries in the “boot block” to find the file name
  uint32_t index;

  if (fname == NULL){
    return -1;
  }

  //Empty string, do nothing but error
  if(fname[0] == '\0')
    return -1;

  //locate directory entry corresponding to the name passed in as argument
  for (index = 0; index < NUM_FILES; index++){

    //if one character does not match, skip to next directory entry
    if (strncmp((int8_t*) fname, ((boot_block_t*) fs_ptr)->direntries[index].filename, FILENAME_LEN) == 0){

      //Populates the dentry parameter -> file name, file type, inode number
      read_dentry_by_index(index, dentry);

      //file found so return success
      return 0;
    }
  }

  //file not found so return -1
  return -1;
}

/* read_dentry_by_index
 * DESCRIPTION: reads directory entry by index
 * INPUTS: index - index in boot block of directory entry
 *				 dentry - directory entry to copy into
 * OUTPUTS: corresponding directory entry stored in dentry
 * RETURN VALUE: int 0 on success, int -1 on failure
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
  //check for invalid index
  if (index >= NUM_FILES || index < 0){
      return -1;
  }

  //copy dentry
  *dentry = ((boot_block_t*) fs_ptr)->direntries[index];

  return 0;
}

/* read_data
 * DESCRIPTION: reads data from a file
 * INPUTS: inode - inode number corresponding to a file
 * 				 offset - offset in bytes to start reading the file from
 *				 buf - buffer to read into
 *				 length - maximum number of bytes to copy
 * OUTPUTS: data copied into buf
 * RETURN VALUE: number of bytes successfully read into buffer
 *							 or -1 if invalid inode number or data block number number encountered
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
  int32_t count, index_in_inode, index_in_data_block, data_block_num, done;
  inode_block_t* inode_block;
  uint8_t* data_block;

  //if inode number is out of range, return -1
  if (inode >= ((boot_block_t*) fs_ptr)->inode_count){
    return -1;
  }

  //initialize count of # bytes read and placed in the buffer
  count = 0;

  //get inode block
  inode_block = (inode_block_t*) (fs_ptr + NUM_B_IN_FOUR_KB * (inode + 1));

  //if trying to begin reading beyond end of file, return -1
  if (offset >= inode_block->length){
    return count;
  }

  //calculate index into inode of data block
  index_in_inode = offset / NUM_B_IN_FOUR_KB;

  //initialize index in data block
  index_in_data_block = offset % NUM_B_IN_FOUR_KB;

  //set flag
  done = 0;

  //while not done reading into buf,
  while (done == 0){

    //calculate data block number
    data_block_num = inode_block->data_block_num[index_in_inode];

    //check for invalid block data number
    if (data_block_num >= ((boot_block_t*) fs_ptr)->data_count){
      return -1;
    }

    //get data block
    data_block = (uint8_t*) (fs_ptr + NUM_B_IN_FOUR_KB * (((boot_block_t*) fs_ptr)->inode_count + data_block_num + 1));

    //while not yet read length # of bytes AND not yet reached end of data block
    //AND not yet reached end of file, read into buf
    while (count < length && index_in_data_block < NUM_B_IN_FOUR_KB && offset < inode_block->length){
      buf[count] = data_block[index_in_data_block];
      index_in_data_block++;
      offset++;
      count++;
    }

    //if read the maximum number of bytes into buf or going out of bounds of the file,
    //end prematurely
    if (count >= length || offset >= inode_block->length){
      done = 1;
    }

    //reset index into data block as a new block is now going to be read
    index_in_data_block = 0;

    //look at next data block
    index_in_inode++;
  }

  //return # of bytes read into buf
  return count;
}

/* init_vfs
 * DESCRIPTION: sets virtual file system pointer for use in vfs
 * INPUTS: int that will be treated as pointer to file system
 * OUTPUTS: sets fs_ptr, populates entries of blank
 * RETURN VALUE: none
 * SIDE EFFECTS: sets fs_ptr
 */
void init_vfs(uint32_t ptr)
{
  fs_ptr = (uint8_t *) ptr;

  // set values of blank file array entry - done here to ensure it happens first
  blank.file_ops_table_ptr = 0;
  blank.inode_num = 0;
  blank.file_position = 0;
  blank.flags = FILE_AVAIL;
}

/* init_file_array
 * DESCRIPTION: populates file array with all blank entries
 * INPUTS: none
 * OUTPUTS: sets all entries in file array to blanks
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 */
void init_file_array(file_t * file_array)
{
  uint32_t i;
  // initialize file_array to default values
  for (i = 0; i < NUM_MAX_OPEN_FILES; i++)
  {
    file_array[i] = blank;
  }

  // stdin
  file_array[0].file_ops_table_ptr = (int32_t) terminal_ops;
  file_array[0].inode_num = 0;
  file_array[0].file_position = 0;
  file_array[0].flags = FILE_OCCUP;

  // stdout
  file_array[1].file_ops_table_ptr = (int32_t) terminal_ops;
  file_array[1].inode_num = 0;
  file_array[1].file_position = 0;
  file_array[1].flags = FILE_OCCUP;
}
