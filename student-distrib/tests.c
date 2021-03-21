#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

#define PASS 1
#define FAIL 0
#define END_OF_EXCEPTION 0x20

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
    for (i = 0; i < NUM_VEC; ++i){
        if ( i<20 && (idt[i].offset_15_00 == NULL) &&   // test first 20 vec we set
            (idt[i].offset_31_16 == NULL)){
            printf("1");
            assertion_failure();
            result = FAIL;
        }
        if(i < END_OF_EXCEPTION){
            if(idt[i].dpl != 0 || idt[i].size != 1 || idt[i].present != 1 || idt[i].seg_selector != KERNEL_CS){
                printf("2");
                assertion_failure();
                result = FAIL;
            }
        }
        else{
            if(idt[i].dpl != 0 || idt[i].size != 1 || idt[i].present != 0 || idt[i].seg_selector != KERNEL_CS){
                printf("3");
                assertion_failure();
                result = FAIL;
            }
        }
    }

    return result;
}

/* Page Structure Test
 *
 * Check every content in paging structures
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging set up
 * Files: boot.S, kernel.C, x86_desc.S
 */
int page_test() {
    TEST_HEADER;

    int result = PASS;
    int i;
    unsigned long tmp;
    uint32_t* tmp_cnt = (uint32_t*) page_directory;
    uint32_t* tmp_cnt0 = (uint32_t*) page_table0;

    // CR3
    asm volatile ("movl %%cr3, %0"
    : "=r" (tmp)
    :
    : "memory", "cc");
    if ((PDE (*) [1024]) tmp != &page_directory) {
        result = FAIL;
        printf("Incorrect CR3!");
    }

    // CR4
    asm volatile ("movl %%cr4, %0"
    : "=r" (tmp)
    :
    : "memory", "cc");
    if ((tmp & 0x10) == 0) {
        result = FAIL;
        printf("Incorrect CR4!");
    }

    // CR0
    asm volatile ("movl %%cr0, %0"
    : "=r" (tmp)
    :
    : "memory", "cc");
    if ((tmp & 0x80000000) == 0) {
        result = FAIL;
        printf("Incorrect CR0!");
    }

    // page directory
    if ((PTE (*) [1024]) ( *tmp_cnt & 0xFFFFF000) != &page_table0) {
        printf("Incorrect page directory: 0");
        result = FAIL;
    }
    if ((tmp_cnt[1] & 0x00400000) != 0x00400000) {
        printf("Incorrect page directory: 1");
        result = FAIL;
    }

    // page table
    for (i = 0; i < 0xB8; ++i){ // start page of video memory
        if (i == 0xB8) {    // 0xB8 is the start of video memory
            if ((tmp_cnt0[i] & 0x000B8003) != 0x000B8003){  // logical and physical memory should match
                printf("Incorrect page table at %d", i);
                result = FAIL;
            }
            continue;
        }
        if ( tmp_cnt0[i] != 0){
            printf("Incorrect page table at %d", i);
            result = FAIL;
        }
    }

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

/* Deference Test
 *
 * Deference to see if handled correctly
 * Inputs: None
 * Outputs: None
 * Side Effects: Cause paging exception if paging is turning on
 * Coverage: paging exception
 * Files: boot.S, kernel.C, x86_desc.S
 */
int dereference_test() {
    TEST_HEADER;
    unsigned long a = 0;
    unsigned long b = *((unsigned long *)a);
    // check if dereference works
    unsigned long* test_ptr = &a;
    if (*test_ptr != a) {
        printf("*test_ptr (0) = %u", a);
        return FAIL;
    }
    // should Never get here
    printf("*0 = %u", b);
    return FAIL;
}

/* RTC Test
 *
 * receive 1024 interrupts
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: RTC interrupts
 * Files: kernel.c
 */
int rtc_test() {
    TEST_HEADER;

    int ref = get_rtc_counter();
    int ctr;
    printf("Count for 1 sec...\n");
    while(1) {
        ctr = get_rtc_counter();
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
    TEST_OUTPUT("rtc_test", rtc_test());
    TEST_OUTPUT("page_test", page_test());
    //TEST_OUTPUT("div0_test", div0_test());
	//TEST_OUTPUT("dereference_test", dereference_test());
}
