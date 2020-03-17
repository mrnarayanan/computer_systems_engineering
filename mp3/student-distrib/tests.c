// #include "tests.h"
// #include "x86_desc.h"
// #include "i8259.h"
// #include "lib.h"
// #include "rtc.h"
// #include "terminal.h"
// #include "types.h"
// #include "vfs.h"
//
// #define PASS 1
// #define FAIL 0
//
// /* format these macros as you see fit */
// #define TEST_HEADER
// 	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
// #define TEST_OUTPUT(name, result)	
// 	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");
//
// static inline void assertion_failure(){
// 	/* Use exception #15 for assertions, otherwise
// 	   reserved by Intel */
// 	asm volatile("int $15");
// }
//
// //#define ENABLE_KERNEL_CRASH_TESTS
//
// /* Checkpoint 1 tests */
//
// /* IDT Test - Example
//  *
//  * Asserts that first 10 IDT entries are not NULL
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: Load IDT, IDT definition
//  * Files: x86_desc.h/S
//  */
// int idt_test(){
// 	TEST_HEADER;
//
// 	int i;
// 	int result = PASS;
// 	for (i = 0; i < 10; ++i){
// 		if ((idt[i].offset_15_00 == NULL) &&
// 			(idt[i].offset_31_16 == NULL)){
// 			assertion_failure();
// 			result = FAIL;
// 		}
// 	}
//
// 	return result;
// }
//
// // add more tests here
//
// /*
//  * irq_test
//  *   DESCRIPTION: enables and disables multiple interrupts to make sure
//  *                 enable_irq and disable_irq work properly
//  *   INPUTS: none
//  *   OUTPUTS: changes masks on the 2 PICs
//  *   SIDE EFFECTS: Disables all interrupts and masks all IRQs
//  *   COVERAGE: enable_irq, disable_irq
//  *   FILES: i8259.c/h
//  */
// int irq_test(){
// 	TEST_HEADER;
// 	cli();                                           //Disable interrupts for test
//
// 	int result = PASS;
// 	enable_irq(1);
// 	disable_irq(1);
//
// 	disable_irq(0); disable_irq(0); disable_irq(0);  //Multiple disables
// 	enable_irq(8); enable_irq(8); enable_irq(8);     //Multiple enables
//
// 	enable_irq(20);                                  //Out of bounds check
// 	disable_irq(20);
//
// 	//Run through all valid IRQ numbers
// 	int i = 0;
// 	for(i = 0; i < 16; i++){
// 		enable_irq(i);
// 		disable_irq(i);
// 	}
// 	return result;
// }
//
// /*
//  * paging_successful_test
//  *   DESCRIPTION: All of these dereferences should be successful. Any page errors
//  *                  indicate there is an issue with paging
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   SIDE EFFECTS: none
//  *   COVERAGE: basic paging support
//  *   FILES: paging.h/c, paging_asm.S
//  */
// int paging_successful_test(){
// 	TEST_HEADER;
//
// 	int result = PASS;
//
// 	int a = 0;
// 	int* ptr_a = &a;
// 	*ptr_a = 5;
// 	if(a != 5){
// 		assertion_failure();
// 		result = FAIL;
// 	}
//
// 	int* video_info = (int *) 0xB8000;
// 	*video_info = 'Q'; //First video location should be a Q
// 	if(*video_info != 'Q'){
// 		assertion_failure();
// 		result = FAIL;
// 	}
//
// 	return result;
// }
//
// /*
//  * paging_failure_test
//  *   DESCRIPTION: Tries to dereference invalid pages. All should result in page
//  *                  fault. If test returns, then paging is not set up.
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   SIDE EFFECTS: Causes a page fault
//  *   COVERAGE: basic paging support
//  *   FILES: paging.h/c, paging_asm.S
//  */
// int paging_failure_test(){
// 	TEST_HEADER;
//
// 	int *a = (int*) 0x0;
// 	*a = 10;   //Dereference NULL
//
// 	a = (int*) 0xF0000;
// 	*a = 10;  //Dereference after video memory
//
// 	a = (int*) 0xFFFFFF00;
// 	*a = 10;  //Dereference after kernel
//
// 	return FAIL;
// }
//
// /*
//  * rtc_interrupt_test
//  *   DESCRIPTION: Check that RTC interrupts are being recieved
//  *   INPUTS: none
//  *   OUTPUTS: writes garbage to screen
//  *   SIDE EFFECTS: enables interrupts, disables RTC after test completes
//  *   COVERAGE: rtc_init, rtc_handler
//  *   FILES: rtc.h/c, interrupthandler.S
//  */
// int rtc_interrupt_test(){
// 	TEST_HEADER;
//
// 	rtc_init();
// 	enable_irq(8);
// 	volatile unsigned long a = 0;
// 	while(a < 1000000000) a++;  //pause to allow time to read output
//
// 	sti();    //Enable interrupts for testing
// 	a = 0;
// 	while (a < 500000000) a++; //pause to allow time for interrupts
// 	disable_irq(8); //Turn off RTC interrupts
// 	clear(); // Clear the screen
//
// 	return PASS;
// }
//
// /*
//  * exception_check_test
//  *   DESCRIPTION: Check that IDT Exceptions are set up properly
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   SIDE EFFECTS: all of these should kernel panic
//  *   COVERAGE: IDT, exception_handler
//  *   FILES: exception_handler.c/h ,x86_desc.h/S
//  */
// int exception_check_test(){
// 	TEST_HEADER;
//
// 	int a = 1;
// 	int b = 10/(--a);  //Call INT0
// 	if(b == 0) assertion_failure();
//
// 	assertion_failure();   //Call INT15
//
// 	return FAIL;
// }
//
// /*
//  * system_trap_test
//  *   DESCRIPTION: Check that a system trap is set up kinda sorta
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   SIDE EFFECTS: Prints a message to the screen
//  *   COVERAGE: IDT, system traps
//  *   FILES: interrupt_handler.c/h, interrupthandler.S, x86_desc.h/S
//  */
// int system_trap_test(){
// 	TEST_HEADER;
//   asm volatile("int $0x80");
// 	return PASS;
// }
//
// /* Checkpoint 2 tests */
//
// //This suite of tests checks for properly working terminal drivers
// /*
//  * termainl_null_check
//  *   DESCRIPTION: pass invalid pointers to the various methods and see if anything breaks
//  */
// int terminal_driver_null_test(){
// 		TEST_HEADER;
// 		// char c;
// 		// uint32_t retval = terminal_read(NULL, TERMINAL_BUFFER_SIZE, &active_terminal);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		// retval = terminal_read(&c, 1, NULL);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		// retval = terminal_write(NULL, TERMINAL_BUFFER_SIZE, &active_terminal);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		// retval = terminal_write(&c, 1, NULL);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
//
// 		return PASS;
// }
//
// /*
//  * terminal_open_close_check
//  *   DESCRITPION: These methods do nothing, check they do nothing
//  */
// int terminal_open_close_check(){
// 		TEST_HEADER;
// 		int32_t retval = terminal_open();
// 		if(retval != 0){assertion_failure(); return FAIL;}
// 		retval = terminal_close();
// 		if(retval != 0){assertion_failure(); return FAIL;}
//
// 		return PASS;
// }
//
// /*
//  * terminal_write_test
//  *   DESCRIPTION: check that terminal_write works under most circumstnaces
//  */
// int terminal_write_test(){
// 		TEST_HEADER;
// 		// int32_t retval;
// 		// printf("Test case 1: This should print nothing:\n");
// 		// retval = terminal_write("A Test of Nothing", 0, &active_terminal);
// 		// if(retval != 0){assertion_failure(); return FAIL;}
// 		//
// 		// printf("Test case 2: You should see a single letter:\n");
// 		// retval = terminal_write("A Test of single letter", 1, &active_terminal);
// 		// if(retval != 1){assertion_failure(); return FAIL;}
// 		//
// 		// printf("\nTest case 3: Print with proper buffer size:\n");
// 		// retval = terminal_write("A 18 byte message\n", 18, &active_terminal);
// 		// if(retval != 18){assertion_failure(); return FAIL;}
//
// 		//This test case no longer valid based on piazza discussion
// /*		printf("\nTest case 4: Buffer reported larger than it is\n");
// 		retval = terminal_write("This message is short.\n", TERMINAL_BUFFER_SIZE, &active_terminal);
// 		if(retval != 23){assertion_failure(); return FAIL;}
// */
// 		return PASS;
// }
//
// /*
//  * terminal_read_test
//  *   DESCRIPTION: check that terminal_read works under most circumstances
//  *   SIDE EFFECTS: Enables keyboard interrupts
//  */
// int terminal_read_test(){
// 		TEST_HEADER;
// 		// char buffer[1000] = {0};
// 		// int32_t retval;
// 		//
// 		// printf("Test case 1: You shouldn't have a chance to write anything:\n");
// 		// retval = terminal_read(buffer, 0, &active_terminal);
// 		// if(retval != 0){assertion_failure(); return FAIL;}
// 		//
// 		// printf("Test case 2: Only a single letter. You should only see a single character echoed.\n");
// 		// retval = terminal_read(buffer, 1, &active_terminal);
// 		// printf("\n"); if(retval != 1){assertion_failure(); return FAIL;}
// 		// terminal_write(buffer, 1, &active_terminal);
// 		//
// 		// printf("Test case 3: Extra large input. Please don't stop typing until you hit max characters:\nPrompt> ");
// 		// retval = terminal_read(buffer, 1000, &active_terminal);
// 		// if(retval != 128){assertion_failure(); return FAIL;}
// 		// terminal_write(buffer, retval, &active_terminal);
// 		//
// 		// printf("Testcase 4: Your choice. Just type at the prompt and the system will echo your input.\n");
// 		// uint8_t i;
// 		// for(i = 0; i < 5; i++){
// 		// 		printf("Prompt> ");
// 		// 		retval = terminal_read(buffer, 1000, &active_terminal);
// 		// 		terminal_write(buffer, retval, &active_terminal);
// 		// }
//
// 		return PASS;
// }
// /*
//  * rtc_open_test
//  *   DESCRIPTION: Initialize and open the RTC device
//  *   SIDE EFFECTS: sets interrupt rate to 2 Hertz
//  */
// int rtc_open_test(){
// 		TEST_HEADER;
// 		int32_t retval = rtc_open();
// 		if(retval != 0){assertion_failure(); return FAIL;}
// 		//Make sure it can handle repeat calls
// 		retval = rtc_open();
// 		if(retval != 0){assertion_failure(); return FAIL;}
// 		retval = rtc_open();
// 		if(retval != 0){assertion_failure(); return FAIL;}
//
// 		return PASS;
// }
//
// /*
//  * rtc_write_parameter_test
//  *   DESCRIPTION: check various values for RTC interrupt rate
//  *   SIDE EFFECTS: modifies RTC interrupt rate
//  */
// int rtc_write_parameter_test(){
// 		TEST_HEADER;
//
// 		// int32_t rate = -1;
// 		// int32_t retval = rtc_write(&rate, 4);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 0;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 1;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 2;
// 		// retval = rtc_write(&rate, 2);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 2;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != 4){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 4;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != 4){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 16;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != 4){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 23;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 512;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != 4){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 1024;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != 4){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 2048;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 8192;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != -1){assertion_failure(); return FAIL;}
// 		//
// 		// rate = 2;
// 		// retval = rtc_write(&rate, 4);
// 		// if(retval != 4){assertion_failure(); return FAIL;}
//
// 		return PASS;
//
// }
//
// /*
//  * rtc_read_test
//  *   DESCRIPTION: Waits at various intervals with the RTC timing
//  *   SIDE EFFETS: Writes to screen, modifies RTC rate
//  */
// int rtc_read_test(){
// 		TEST_HEADER;
// 		// int32_t retval;
// 		// int32_t rate = 2;
// 		// int32_t counter = 0;
// 		// enable_irq(8);
// 		// rtc_write(&rate, 4);
// 		//
// 		// for(counter = 0; counter < 6; counter++){
// 		// 		retval = rtc_read(&rate, 4); //Wait a few seconds
// 		// 		if(retval != 0){assertion_failure(); return FAIL;}
// 		// }
// 		//
// 		// clear_terminal(&active_terminal);
// 		//
// 		// for(counter = 0; counter < 10; counter++){
// 		// 		rtc_read(&rate, 4);
// 		// 		terminal_putc('1', ATTRIB, &active_terminal);
// 		// }
// 		//
// 		// clear_terminal(&active_terminal);
// 		// rate = 4;
// 		// rtc_write(&rate, 4);
// 		// for(counter = 0; counter < 10; counter++){
// 		// 		rtc_read(&rate, 4);
// 		// 		terminal_putc('1', ATTRIB, &active_terminal);
// 		// }
// 		//
// 		// clear_terminal(&active_terminal);
// 		// rate = 8;
// 		// rtc_write(&rate, 4);
// 		// for(counter = 0; counter < 20; counter++){
// 		// 		rtc_read(&rate, 4);
// 		// 		terminal_putc('1', ATTRIB, &active_terminal);
// 		// }
// 		//
// 		// clear_terminal(&active_terminal);
// 		// rate = 16;
// 		// rtc_write(&rate, 4);
// 		// for(counter = 0; counter < 40; counter++){
// 		// 		rtc_read(&rate, 4);
// 		// 		terminal_putc('1', ATTRIB, &active_terminal);
// 		// }
// 		//
// 		// clear_terminal(&active_terminal);
// 		// rate = 32;
// 		// rtc_write(&rate, 4);
// 		// for(counter = 0; counter < 100; counter++){
// 		// 		rtc_read(&rate, 4);
// 		// 		terminal_putc('1', ATTRIB, &active_terminal);
// 		// }
//
// 		return PASS;
// }
//
// /*
//  * vfs_tests
//  *   DESCRIPTION:
//  *   SIDE EFFETS:
//  */
// int vfs_tests()
// {
// 	TEST_HEADER;
// // 	int8_t buf[10000];
// // 	uint8_t * filename = (uint8_t *)".";
// //
// // 	if (directory_open(filename) == -1)
// // 		return FAIL;
// // 	int32_t read = -1;
// // 	do{
// // 		read = directory_read(2, buf, 10000);
// // 		terminal_write(buf, read, &active_terminal);
// // 		printf("\n");
// // 	} while(read != 0);
// //
// // 	directory_close(2);
// // uint32_t rate = 2;
// // rtc_write(&rate, 4);
// // 	for(rate = 0; rate < 10; rate++){
// // 		rtc_read(&rate, 2);
// // 	}
// //
// // filename = (uint8_t *)"frame0.txt";
// // 	if (file_open(filename) == -1)
// // 		return FAIL;
// // 	read = file_read(2, buf, 10000);
// // 	if(read != 187) return FAIL;
// // 	terminal_write(buf, read, &active_terminal);
// // 	file_close(2);
// //
// // 	for(rate = 0; rate < 5; rate++){
// // 		rtc_read(&rate, 2);
// // 	}
// //
// // 	for(rate = 0; rate < 5; rate++){
// // 		rtc_read(&rate, 2);
// // 	}
// //
// // 	filename = (uint8_t *)"ls";
// // 	if(file_open(filename) == -1)
// // 		return FAIL;
// // 	read = file_read(2, buf, 10000);
// // 	if(read != 5349)
// // 		return FAIL;
// // 	terminal_write(buf, read, &active_terminal);
// // //	file_close(2);
// //
// // 	for(rate = 0; rate < 5; rate++){
// // 		rtc_read(&rate, 2);
// // 	}
// //
// // 	filename = (uint8_t *)"verylargetextwithverylongname.txt";
// // 	if(file_open(filename) != -1)
// // 		return FAIL;
// //
// // 	filename = (uint8_t *)"verylargetextwithverylongname.tx";
// // 	if(file_open(filename) == -1)
// // 		return FAIL;
// // 	do{
// // 		read = file_read(3, buf, 90);
// // 		terminal_write(buf, read, &active_terminal);
// // 	} while (read != 0);
// //
// // 	file_close(2);
// // 	file_close(3);
// //
// // 	filename = (uint8_t *)"Idontexist";
// // 	if(file_open(filename) != -1)
// // 		return FAIL;
//
// 	return PASS;
// }
// /* Checkpoint 3 tests */
// /* Checkpoint 4 tests */
// /* Checkpoint 5 tests */
//
//
// /* Test suite entry point */
// void launch_tests(){
// 	volatile unsigned long a = 0;
// 	while(a < 250000000) a++;  //pause to allow time to read output
//
// 	clear();                    //clear the screen
//
// 	TEST_OUTPUT("idt_test", idt_test());
// 	// launch your tests here
// 	TEST_OUTPUT("iqr_test", irq_test());
// 	TEST_OUTPUT("paging_successful_test", paging_successful_test());
// 	TEST_OUTPUT("system_trap_test", system_trap_test());
// //	TEST_OUTPUT("rtc_interrupt_test", rtc_interrupt_test());// Test is no longer needed
//
// 	printf("\nCheckpoint 2 Tests:\n");
//
// 	//enable keyboard and RTC IRQs and interrupts
// 	sti();
// 	enable_irq(1);
// 	enable_irq(8);
//
// 	TEST_OUTPUT("rtc_open_test", rtc_open_test());
// 	TEST_OUTPUT("rtc_write_parameter_test", rtc_write_parameter_test());
// 	TEST_OUTPUT("rtc_read_test", rtc_read_test());
// 	TEST_OUTPUT("file open/read", vfs_tests());
//
// 	TEST_OUTPUT("terminal_driver_null_test", terminal_driver_null_test());
// 	TEST_OUTPUT("terminal_open_close_check", terminal_open_close_check());
// 	TEST_OUTPUT("terminal_write_test", terminal_write_test());
// 	TEST_OUTPUT("terminal_read_test", terminal_read_test());
//
// 	#ifdef ENABLE_KERNEL_CRASH_TESTS
// 	TEST_OUTPUT("paging_failure_test", paging_failure_test());
//   TEST_OUTPUT("exception_check_test", exception_check_test());
// 	#endif //ENABLE_KERNEL_CRASH_TESTS
// }
