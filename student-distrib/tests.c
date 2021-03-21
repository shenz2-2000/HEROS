#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");
extern int get_rtc_counter();

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

int page_test() {
    TEST_HEADER;

    int result = PASS;
    int i;
    unsigned long tmp;

    // Check CR3 content
    asm volatile ("movl %%cr3, %0"
    : "=r" (tmp)
    :
    : "memory", "cc");
    if ((PDE (*) [1024]) tmp != &page_directory) {
        printf("CR3 not correct");
        result = FAIL;
    }

//    // Check CR0
//    asm volatile ("movl %%cr0, %0"
//    : "=r" (tmp)
//    :
//    : "memory", "cc");
//    if ((tmp & 0x80000000) == 0) {
//        printf("Paging is not enabled");
//        result = FAIL;
//    }
//
//    // Check kernel page directory
//    if ((PTE (*) [1024]) (page_directory[0] & 0xFFFFF000) != &page_table0) {
//        printf("Paging directory entry 0 is not correct");
//        result = FAIL;
//    }
//    if ((page_directory[1] & 0x00400000) != 0x00400000) {
//        printf("Paging directory entry 1 is not correct");
//        result = FAIL;
//    }
//
//    // Check kernel page table 0
//    for (i = 0; i < VIDEO_MEMORY_START_PAGE; ++i){
//        if (kernel_page_table_0.entry[i] != 0){
//            printf("Paging table entry %d is not correct", i);
//            result = FAIL;
//        }
//    }
//    if ((kernel_page_table_0.entry[VIDEO_MEMORY_START_PAGE] & 0x000B8003) != 0x000B8003){
//        printf("Paging table entry for video memory is not correct");
//        result = FAIL;
//    }
//    for (i = VIDEO_MEMORY_START_PAGE + 1; i < KERNEL_PAGE_TABLE_SIZE; ++i){
//        if (kernel_page_table_0.entry[i] != 0){
//            printf("Paging table entry %d is not correct", i);
//            result = FAIL;
//        }
//    }
//
//    int* i_ptr = &i;
//    if (*i_ptr != i) {
//        printf("Dereference i error");
//        result = FAIL;
//    }

    return result;
}

/* Zero division Test
 *
 * Divide 0 to see if handled correctly
 * Inputs: None
 * Outputs: None
 * Side Effects: Cause zero-division error
 * Coverage: Divide Error exception
 * Files: boot.S, kernel.C
 */
int div0_test() {
    TEST_HEADER;
    unsigned long a = 0;
    unsigned long b = 1 / a;

    // should never get here
    printf("1 / 0 = %u", b);
    return FAIL;
}

/* NULL Deference Test
 *
 * Deference 0 to see if handled correctly
 * Inputs: None
 * Outputs: None
 * Side Effects: Cause paging exception if paging is turning on
 * Coverage: paging exception
 * Files: boot.S, kernel.C
 */
int dereference_null_test() {
    TEST_HEADER;
    unsigned long i = 0;
    unsigned long j = *((unsigned long *)i);

    // should Never get here
    printf("*0 = %u", j);
    return FAIL;
}


int rtc_test() {
    TEST_HEADER;

    int ref = get_rtc_counter();
    int ctr;
    printf("Count for 1 sec...\n");
    while(1) {
        cli(); {
            ctr = get_rtc_counter();
        }
        sti();
        if (ctr - ref > 1024) {
            break;
        }
    }
    return PASS;
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
    // tests cp1
    clear();
    reset_screen();
	TEST_OUTPUT("idt_test", idt_test());
    //TEST_OUTPUT("rtc_test", rtc_test());
    TEST_OUTPUT("page_test", page_test());
    //TEST_OUTPUT("div0_test", div0_test());
	TEST_OUTPUT("dereference_null_test", dereference_null_test());
}
