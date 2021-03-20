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
    printf("No paging test.");
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
    TEST_OUTPUT("rtc_test", rtc_test());

    //TEST_OUTPUT("div0_test", div0_test());
	TEST_OUTPUT("dereference_null_test", dereference_null_test());
}
