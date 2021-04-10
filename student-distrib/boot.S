# boot.S - start point for the kernel after GRUB gives us control
# vim:ts=4 noexpandtab

#define ASM     1

#include "multiboot.h"
#include "x86_desc.h"

.text

    # Multiboot header (required for GRUB to boot us)
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

# Entrypoint to the kernel
.globl start, _start

.align 4
start:
_start:
    # Make sure interrupts are off
    cli
    jmp     continue

continue:

    # using gdtr_ptr defined in x86_desc.S to load GDT
    lgdt gdtr_ptr

    # Load CS with the new descriptor value
    ljmp    $KERNEL_CS, $keep_going

keep_going:
    # Set up ESP so we can have an initial stack
    movl    $0x800000, %esp

    # Set up the rest of the segment selector registers
    movw    $KERNEL_DS, %cx
    movw    %cx, %ss
    movw    %cx, %ds
    movw    %cx, %es
    movw    %cx, %fs
    movw    %cx, %gs

    # Push the parameters that entry() expects (see kernel.c):
    # eax = multiboot magic
    # ebx = address of multiboot info struct
    pushl   %ebx
    pushl   %eax

    # Jump to the C entrypoint to the kernel.
    call    entry

    # We'll never get back here, but we put in a hlt anyway.
halt:
    hlt
    jmp     halt

# Macros for save and restore all registers
.macro SAVE_ALL_REGS
	pushf
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
.endm

.macro RESTORE_ALL_REGS
    popl %eax
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popf
.endm

.macro SAVE_ALL_REGS_SYS
	pushf
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
.endm

.macro RESTORE_ALL_REGS_SYS
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popf
.endm

# here we define the exception handler with linkage

/* NOTE: The following functions are in the same format so we only use one interface
 * exception_handler_num
 *   DESCRIPTION: call the corresponding exception handler
 *   INPUTS: num -- the exception number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: jump to the corresponding exception handler
 */
.globl exception_handler_0
exception_handler_0:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $0
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_1
exception_handler_1:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $1
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_2
exception_handler_2:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $2
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_3
exception_handler_3:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $3
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_4
exception_handler_4:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $4
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_5
exception_handler_5:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $5
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_6
exception_handler_6:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $6
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_7
exception_handler_7:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $7
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_8
exception_handler_8:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $8
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_9
exception_handler_9:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $9
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_10
exception_handler_10:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $10
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_11
exception_handler_11:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $11
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_12
exception_handler_12:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $12
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_13
exception_handler_13:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $13
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_14
exception_handler_14:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $14
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_15
exception_handler_15:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $15
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_16
exception_handler_16:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $16
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_17
exception_handler_17:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $17
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_18
exception_handler_18:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $18
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


.globl exception_handler_19
exception_handler_19:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $19
         # ---------------------call handler function------------------
         call naive_exception_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET


/* NOTE: The following functions are in the same format so we only use one interface
 * interrupt_entry_num
 *   DESCRIPTION: call the corresponding interrupt handler
 *   INPUTS: irq -- the irq number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: jump to the corresponding interrupt handler
 */
 .globl interrupt_entry_1
interrupt_entry_1:
    SAVE_ALL_REGS
    call keyboard_interrupt_handler
    pushl $1  # push irq number as argument
    call send_eoi
    addl $4, %esp 
    RESTORE_ALL_REGS
    iret  # return from interrupt 


.globl interrupt_entry_8
interrupt_entry_8:
    SAVE_ALL_REGS
    call rtc_interrupt_handler
    pushl $8  # push irq number as argument
    call send_eoi
    addl $4, %esp 
    RESTORE_ALL_REGS
    iret  # return from interrupt 

# System call handler is in the same format as exception handler
# naive system call handler
.globl exception_handler_128
exception_handler_128:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $128
         # ---------------------call handler function------------------
         call naive_system_call_handler
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET



/*
 * sys_call_linkage
 *   DESCRIPTION: jump to the correct system call handler
 *   INPUTS: num and parameters -- the system call number and other parameters, all in registers
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: jump to the corresponding system call handler
 */

.globl sys_call_linkage
sys_call_linkage:

           # --------------save all registers--------------
           SAVE_ALL_REGS_SYS

           # ----------------check sanity-----------------
           cmpl $MAXIMUM_SYS_CALL_NUM, %eax
           jae  invalid_sys_call

           # ----------------jump to real func------------
           call *sys_jump_table(,%eax,4)
           jmp sys_call_finish

invalid_sys_call:

           # TODO: what should we do here?

sys_call_finish:
           # --------------restore information--------------
           RESTORE_ALL_REGS_SYS
           IRET

sys_jump_table:
          .long dummy_sys_call
          .long halt_sys_call
          .long execute_sys_call
          .long read_sys_call
          .long write_sys_call
          .long open_sys_call
          .long close_sys_call
          .long get_args_sys_call
          .long vidmap_sys_call
          .long set_handler_sys_call
          .long sig_return_sys_call


/**
 * init_page_register
 * Description: We use this funciton to init the register related to paging
 * Input: None.
 * Output: None.
 * Side effect: Enable paging
 */
.globl init_page_register
init_page_register:

      # enter the function, just a shortcut
      enter $0, $0

      # we set CR3 here
      movl $page_directory, %ecx
      movl %cr3, %eax
      orl  %ecx, %eax
      movl %eax, %cr3

      # we set CR4 here
      movl    %cr4, %ecx
      orl     $0x10, %ecx
      movl    %ecx, %cr4

      # we set CR0 here
      movl %cr0, %ecx
      orl  $0x80000001, %ecx
      movl %ecx, %cr0

      leave
      ret







