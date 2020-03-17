.code32

.data					# section declaration

        # Useful offset constants for accessing members of a
        # struct mp1_blink_struct structure
        LOCATION   = 0
        ON_CHAR    = 2
        OFF_CHAR   = 3
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:

	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret

/*
void mp1_rtc_tasklet(unsigned long);
Updates status of game
Inputs: None (argument not used)
Outputs: Updates values in each struct in linked list 
*/
mp1_rtc_tasklet:
  /* stack setup */
  pushl %ebp
  movl  %esp, %ebp
  pushl %esi
  pushl %edi
  pushl %ebx

  xorl  %ebx, %ebx
  movl  mp1_list_head, %esi # ESI points to each struct
  traverse:
  cmpl  $0, %esi # check if pointing to NULL
  je    task_end
  movw  COUNTDOWN(%esi), %bx
  decw  %bx
  movw  %bx, COUNTDOWN(%esi)
  cmpw $0, %bx
  jne   load_next
  movw  STATUS(%esi), %bx
  cmpw $0, %bx
  je    zero_case # if STATUS was 0
  /* one_case */
  movw  $0, STATUS(%esi)
  movw  OFF_LENGTH(%esi), %bx
  movw  %bx, COUNTDOWN(%esi)
  movzwl  (%esi), %eax
  shl   $1, %eax # left shift by 1 is multiplying by 2
  xorl  %ecx, %ecx # clear ecx
  movb  OFF_CHAR(%esi), %cl
  call  mp1_poke
  jmp   load_next

  zero_case:
  movw  $1, STATUS(%esi)
  movw  ON_LENGTH(%esi), %bx
  movw  %bx, COUNTDOWN(%esi)
  movzwl  (%esi), %eax
  shl   $1, %eax # left shift by 1 is multiplying by 2
  xorl  %ecx, %ecx # clear ecx
  movb  ON_CHAR(%esi), %cl
  call  mp1_poke

  load_next:
  movl  NEXT(%esi), %esi
  jmp   traverse

  /* stack teardown */
  task_end:
  popl %ebx
  popl %edi
  popl %esi
  leave
  ret

/*
int mp1_ioctl(unsigned long arg, unsigned long cmd);
Dispatcher function used to jump to ioctl Core Functions
Inputs: argument for ioctl Core Functions (arg), 
		selection for which ioctl Core Function to jump to (cmd)
Outputs: Returns -1 if invalid cmd 
*/
mp1_ioctl:
  movl  8(%esp), %ecx
  cmpl  $3, %ecx # valid cmd options are between 0-3
  ja    fail
  jmp   *jump_table(,%ecx,4)
  fail:
  movl  $-1, %eax
  ret

/*
int mp1_ioctl_add(unsigned long arg);
Adds a mp1_blink_struct to the linked list
Inputs: user level pointer to a mp1_blink_struct
Outputs: Returns 0 if add was successful, -1 if not successful
*/
mp1_ioctl_add:
  /* stack setup */
  pushl %ebp
  movl  %esp, %ebp
  pushl %esi
  pushl %edi
  pushl %ebx
  movl  8(%ebp), %edi # arg copied to EDI

  /* calling malloc */
  movl  $STRUCT_SIZE, %ebx
  pushl %ebx # argument for malloc
  call  mp1_malloc
  cmpl  $0, %eax
  je    err_add # if malloc returned NULL

  /* calling mp1_copy_from_user */
  pushl %edi # 3 arguments - STRUCT_SIZE already on stack, then from*
  pushl %eax # to*
  call  mp1_copy_from_user
  popl  %esi # pops pointer to malloc'd memory to ESI
  addl  $8, %esp # pops last 2 arguments for copy_from_user off stack
  cmpl  $0, %eax
  jne   clr_add # if copy from user does not return 0

  /* add processes */
  movw  ON_LENGTH(%esi), %cx
  movw  %cx, COUNTDOWN(%esi)
  movw  $1, STATUS(%esi)
  movl  mp1_list_head, %ecx
  movl  %ecx, NEXT(%esi)
  movl  %esi, mp1_list_head
  movzwl  (%esi), %eax
  cmpl  $1999, %eax # max index on screen (80*25-1)
  ja    clr_add # if location is out of bounds
  shl   $1, %eax # left shift by 1 is multiplying by 2
  xorl  %ecx, %ecx # clear ecx
  movb  ON_CHAR(%esi), %cl
  call  mp1_poke
  movl  $0, %eax
  jmp   fin_add

  clr_add: # frees malloc'd memory if error occurred
  pushl %esi
  call  mp1_free
  popl  %esi
  err_add:
  movl  $-1, %eax
  fin_add:
  /* pop local vars off stack by using ESP if nec. */
  popl %ebx
  popl %edi
  popl %esi
  leave
  ret

/*
int mp1_ioctl_remove(unsigned long arg);
Removes a mp1_blink_struct from the linked list
Inputs: integer location value
Outputs: Returns 0 if remove was successful, -1 if no struct with location found 
*/
mp1_ioctl_remove:
  /* stack setup */
  pushl %ebp
  movl  %esp, %ebp
  pushl %esi
  pushl %edi
  pushl %ebx
  movl  8(%ebp), %esi # arg copied to ESI

  movw  %si, %cx
  call  mp1_search
  cmpl  $0, %eax
  je    err_rem # if LOCATION not found in structs
  movl  %eax, %esi # move address of struct to remove to ESI
  movl  mp1_list_head, %edi
  cmpl  %edi, %esi # if head pointer points to our struct (first in list)
  je    first_element

  rem_trav: # loop to find previous struct
  movl  NEXT(%edi), %ebx
  cmpl  %esi, %ebx
  je    prev_found
  movl  NEXT(%edi), %edi
  jmp   rem_trav
  prev_found: # previous struct ptr is in EDI
  movl  NEXT(%esi), %ebx
  movl  %ebx, NEXT(%edi)
  pushl %esi
  call  mp1_free
  popl  %esi
  jmp   rem_suc

  first_element:
  movl  NEXT(%esi), %ebx
  movl  %ebx, mp1_list_head
  pushl %esi
  call  mp1_free
  popl  %esi

  rem_suc: # remove was successful
  movl  $0, %eax
  jmp   fin_rem
  err_rem:
  movl  $-1, %eax
  fin_rem: /* stack teardown */
  popl %ebx
  popl %edi
  popl %esi
  leave
  ret

/*
int mp1_ioctl_find(unsigned long arg);
Gets information about a location
Inputs: pointer to a mp1_blink_struct
Outputs: Returns 0 if find was successful, -1 if no matching location found in list 
*/
mp1_ioctl_find:
  /* stack setup */
  pushl %ebp
  movl  %esp, %ebp
  pushl %esi
  pushl %edi
  pushl %ebx
  movl  8(%ebp), %esi # arg copied to ESI (ptr to user struct)
  cmpl  $0, %esi
  je    err_find # if ptr is NULL
  movw  (%esi), %cx # copy LOCATION into CX, assume valid bc in linked list
  call  mp1_search
  cmpl  $0, %eax
  je    err_find # if struct w/ same LOCATION not found in linked list
  movl  %eax, %edi
  movl  $STRUCT_SIZE, %ebx
  pushl %ebx
  pushl %edi
  pushl %esi
  call  mp1_copy_to_user
  addl  $12, %esp # pop the 3 arguments off stack
  cmpl  $0, %eax
  jne   err_find
  movl  $0, %eax
  jmp   fin_find

  err_find:
  movl  $-1, %eax
  fin_find: /* stack teardown */
  popl %ebx
  popl %edi
  popl %esi
  leave
  ret

/*
int mp1_ioctl_sync(unsigned long arg);
Synchronize a new blink location with an existing one 
Inputs: two location (unsigned short) integers packed as one argument
Outputs: Returns 0 if sync was successful, -1 if not successful
*/
mp1_ioctl_sync:
  /* stack setup */
  pushl %ebp
  movl  %esp, %ebp
  pushl %esi
  pushl %edi
  pushl %ebx
  movl  8(%ebp), %esi # arg copied to ESI
  xorl  %edi, %edi # clear EDI
  movw  %si, %di # DI has second location
  shr   $16, %esi # SI has first location
  movw  %si, %cx
  call  mp1_search
  movl  %eax, %esi # ESI has ptr to first struct
  movw  %di, %cx
  call  mp1_search
  movl  %eax, %edi # EDI has ptr to second struct
  cmpl  $0, %esi
  je    err_sync # if first pointer is NULL
  cmpl  $0, %edi
  je    err_sync # if second pointer is NULL
  xorl  %ebx, %ebx # clearing EBX to use for copy transfers
  movw  ON_LENGTH(%esi), %bx
  movw  %bx, ON_LENGTH(%edi)
  movw  OFF_LENGTH(%esi), %bx
  movw  %bx, OFF_LENGTH(%edi)
  movw  COUNTDOWN(%esi), %bx
  movw  %bx, COUNTDOWN(%edi)
  movw  STATUS(%esi), %bx
  movw  %bx, STATUS(%edi)

  movzwl  (%edi), %eax
  shl   $1, %eax # left shift by 1 is multiplying by 2
  xorl  %ecx, %ecx # clear ecx
  movw  STATUS(%edi), %bx
  cmpw  $0, %bx
  je    s_zero # if STATUS was 0
  /* s_one case */
  movb  ON_CHAR(%edi), %cl
  call  mp1_poke
  jmp   s_suc

  s_zero:
  movb  OFF_CHAR(%edi), %cl
  call  mp1_poke

  s_suc: # sync was successful
  movl  $0, %eax
  jmp   fin_sync

  err_sync:
  movl  $-1, %eax
  
  fin_sync: # stack teardown
  popl %ebx
  popl %edi
  popl %esi
  leave
  ret

/*
unsigned long mp1_search(void);
Register based arguments (not C-style)
Inputs: %cx has location being searched for
Outputs: %eax will have pointer to struct with that location
		will return 0 (NULL) if location not found
*/
mp1_search:
  pushl %esi
  pushl %ebx

  xorl  %ebx, %ebx # clear EBX 
  movl  mp1_list_head, %esi # ESI points to each struct
  travel: # loop to traverse linked list 
  cmpl  $0, %esi # check if pointing to NULL
  je    not_found
  movw  (%esi), %bx
  cmpw  %cx, %bx
  je    found
  movl  NEXT(%esi), %esi
  jmp   travel
  not_found:
  movl  $0, %eax
  jmp   fin_search
  found:
  movl  %esi, %eax
  fin_search:
  popl  %ebx
  popl  %esi
  ret

jump_table:
.long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync

.end
