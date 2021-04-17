# boot.S - start point for the kernel after GRUB gives us control
# vim:ts=4 noexpandtab

#define ASM     1

#include "multiboot.h"
#include "x86_desc.h"
#include "signal_sys_call.h"

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

# for signal mechanism
.macro SET_HW_CONTEXT
    push  %fs
    push  %es
    push  %ds
    pushl %eax
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
.endm


.macro RESTORE_HW_CONTEXT
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
    pop  %ds
    pop  %es
    pop  %fs
    addl $8, %esp
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
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $0 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_1
exception_handler_1:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_2
exception_handler_2:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $2 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_3
exception_handler_3:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $3 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_4
exception_handler_4:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $4 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_5
exception_handler_5:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $5 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_6
exception_handler_6:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $6 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_7
exception_handler_7:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $7 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_8
exception_handler_8:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $8 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_9
exception_handler_9:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $9 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_10
exception_handler_10:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_11
exception_handler_11:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_12
exception_handler_12:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_13
exception_handler_13:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_14
exception_handler_14:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_15
exception_handler_15:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_16
exception_handler_16:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_17
exception_handler_17:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_18
exception_handler_18:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
         IRET


.globl exception_handler_19
exception_handler_19:
         pushl $0 /*push dummy*/
         # ---------------------push the entry number------------------
         pushl $1 /*push irq number*/
         SET_HW_CONTEXT
         # ---------------------call handler function------------------
         call naive_exception_handler
         call check_signal
         # ---------------------restore all registers------------------
         RESTORE_HW_CONTEXT
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
           pushl $0         /*push dummy*/
           pushl $0x80      /*push vector number*/
           # --------------save all registers--------------
           SET_HW_CONTEXT

           # ----------------check sanity-----------------
           cmpl $MAXIMUM_SYS_CALL_NUM, %eax
           jae  invalid_sys_call

           # ----------------jump to real func------------
           call *sys_jump_table(,%eax,4)
           jmp sys_call_finish

invalid_sys_call:

           call *sys_jump_table

sys_call_finish:
           # --------------restore information--------------
           movl %eax, 24(%esp)
           call check_signal
           RESTORE_HW_CONTEXT
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





.globl user_handler_helper
.globl sys_sig_return

// TODO: restore_mask to be written

storage_cur_signal_ptr:
     .long 0
storage_handler_ptr:
     .long 0
storage_hw_ptr:
     .long 0
user_esp:
     .long 0
user_ebp:
     .long 0
kernel_esp:
     .long 0
kernel_ebp:
     .long 0
jump_to_sigreturn_linkage:
     .long 0
hw_context_ptr:
     .long 0

user_hw_context_ptr:
     .long 0
kernel_hw_context_ptr:
     .long 0

signal_return_linkage:
     movl $10, %eax
     INT  $0x80

user_handler_helper:

     # ----------------------Set stack frame for user_handler_helper--------------------

     # save callee-saved register
     pushl %ebp
     pushl %edi
     pushl %esi
     pushl %ebx

     # ebx is the stack frame pointer
     movl 20(%esp), %ebx


     # -----------------------Prepare for calling user_level handler--------------------

     # store the parameters
     movl (%ebx), %edi
     movl %edi, storage_cur_signal_ptr
     movl 4(%ebx), %edi
     movl %edi, storage_handler_ptr
     movl 8(%ebx), %edi
     movl %edi, storage_hw_ptr

     # get user esp and ebp
     movl 20(%edi), %esi
     movl %esi, user_ebp
     movl 60(%edi), %esi
     movl %esi, user_esp

     # store %edi
     movl %edi, hw_context_ptr

     # store kernel stack and load user stack
     movl %esp, kernel_esp
     movl %ebp, kernel_ebp
     movl user_esp, %esp
     movl user_ebp, %ebp

     # --------------------------Set stack frame for user_handler-----------------------

     # setup execute sigreturn linkage to stack
     movl signal_return_linkage, %esi
     movl signal_return_linkage + 3, %edi

     # only 7 bytes for that two instructions
     subl $7, %esp
     movl %esi, (%esp)
     movl %edi, 3(%esp)

     # store the return address for sigreturn linkage and set 4-alignment
     movl %esp, jump_to_sigreturn_linkage
     subl $1, %esp

     # set hard-ware context for the user level handler
     movl hw_context_ptr, %edi
     pushl   64(%edi)    # ss
     pushl   60(%edi)    # esp
     pushl   56(%edi)    # eflags
     pushl   52(%edi)    # cs
     pushl   48(%edi)    # eip
     pushl   44(%edi)    # error
     pushl   40(%edi)    # irq
     pushl   36(%edi)    # fs
     pushl   32(%edi)    # es
     pushl   28(%edi)    # ds
     pushl   24(%edi)    # eax
     pushl   20(%edi)    # ebp
     pushl   16(%edi)    # edi
     pushl   12(%edi)    # esi
     pushl   8(%edi)     # edx
     pushl   4(%edi)     # ecx
     pushl   0(%edi)     # ebx

     # set parameters for C-calling convention
     pushl storage_cur_signal_ptr
     pushl jump_to_sigreturn_linkage

     # update esp on user stack for the IRET of system call linkage
     movl %esp, 60(%edi)

     # call the user_level handler
     movl storage_handler_ptr, %esi
     movl %esi, 48(%edi)

     # now we go back to current system call since IRET's context is set
     movl kernel_esp, %esp
     movl kernel_ebp, %ebp

     popl %ebx
     popl %esi
     popl %edi

     leave
     ret



sys_sig_return:

      # --------------------prepare for context copy-----------------

      # get pointers to hw_context
      movl 64(%esp), %edi
      addl $4, %edi
      movl %esp, %esi
      addl $4, %esi

      # store two hw_context pointers
      movl %esi, kernel_hw_context_ptr
      movl %edi, user_hw_context_ptr

      # --------------------------context copy-----------------------

      xorl %ebx, %ebx

copy_loop:

      movl  (%edi,%ebx,4), %ecx
      movl  %ecx, (%esi,%ebx,4)
      addl  $1,  %ebx
      cmpl  $17, %ebx
      jne   copy_loop

      # -----------------------restore signal mask--------------------

      pushl %ebx
      pushl %ecx
      pushl %eax

      call restore_signal

      popl  %eax
      popl  %ecx
      popl  %ebx

      movl 24(%esi), %eax

      ret



