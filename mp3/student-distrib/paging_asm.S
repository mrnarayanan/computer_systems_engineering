/* includes code from OSDev.org*/

.text
.globl loadPageDirectory
/* loadPageDirectory
 * description: loads the page directory into CR3
 * inputs: unsigned long * - pointer to page directory
 * outputs: none
 * return value: none
 * side effects: set cr3 (loads page directory)
 */
loadPageDirectory:
	push %ebp
	mov %esp, %ebp
	mov 8(%esp), %eax # page directory pointer is first argument
	mov %eax, %cr3		# load paging directory base address into cr3
	mov %ebp, %esp
	pop %ebp
	ret

.text
.globl setPageSize
/* setPageSize
 * description: Correctly sets CR4 to enable paging
 * inputs: none
 * outputs: none
 * return value: none
 * side effects: sets cr4 (PSE and clears PAE)
 */
setPageSize:
	push %ebp
	mov %esp, %ebp
	mov %cr4, %eax
	or $0x00000010, %eax	# set PSE flag for 4MB page sizes
	and $0xFFFFFFDF, %eax	# clear PAE flag to maintain 32-bit addressing
	mov %eax, %cr4
	mov %ebp, %esp
	pop %ebp

.text
.globl enablePaging
/* enablePaging
 * description: Sets the paging enabled flag in CR0
 * inputs: none
 * outputs: none
 * return value: none
 * side effects: set cr0 (enables paging)
 */
enablePaging:
	push %ebp
	mov %esp, %ebp
	mov %cr0, %eax
	or $0x80000001, %eax	# enable PE flag and paging flag
	mov %eax, %cr0
	mov %ebp, %esp
	pop %ebp
	ret
