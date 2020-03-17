#ifndef VFS_H_
#define VFS_H_

#include "types.h"
#include "lib.h"
#include "rtc.h"
#include "pcb.h"

#define FILENAME_LEN 32
#define NUM_FILES 63
#define NUM_B_IN_FOUR_KB 4096
#define FILE_AVAIL 1
#define FILE_OCCUP 0

extern int32_t file_open(const uint8_t* filename);

extern int32_t file_close(int32_t fd);

extern int32_t file_read(int32_t fd, int8_t* buf, int32_t nbytes);

extern int32_t file_write(int32_t fd, const int8_t* buf, int32_t nbytes);

extern int32_t directory_open(const uint8_t* filename);

extern int32_t directory_close(int32_t fd);

extern int32_t directory_read(int32_t fd, int8_t* buf, int32_t nbytes);

extern int32_t directory_write(int32_t fd, const int8_t* buf, int32_t nbytes);

//struct field names taken from lecture notes
//64 bytes total
typedef struct dentry_t{
	int8_t filename[FILENAME_LEN];
	int32_t filetype;
	int32_t inode_num;
	int8_t reserved[24];
} dentry_t;

//4096 bytes total
typedef struct boot_block_t{
	int32_t dir_count;
	int32_t inode_count;
	int32_t data_count;
	int8_t reserved[52];
	dentry_t direntries[63];
} boot_block_t;

//4096 bytes total
typedef struct inode_t{
	int32_t length;
	int32_t data_block_num[1023];
} inode_block_t;

extern int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

extern void init_vfs(uint32_t ptr);

extern int32_t load_program(const uint8_t* filename, uint8_t * ptr);

extern void init_file_array(file_t *file_array);

#endif
