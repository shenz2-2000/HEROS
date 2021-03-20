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


# here we define the exception handler

.macro EXCEPTION_HANDLER num,func_ptr

.globl exception_handler_\num
exception_handler_\num:
         # ----------------------push all registers--------------------
         SAVE_ALL_REGS
         # ---------------------push the entry number------------------
         pushl $\num
         # ---------------------call handler function------------------
         call \func_ptr
         addl $4, %esp
         # ---------------------restore all registers------------------
         RESTORE_ALL_REGS
         IRET
.endm

# setup all 20 naive handlers (only print something to screen and freeze the system)

EXCEPTION_HANDLER 0, naive_exception_handler
EXCEPTION_HANDLER 1, naive_exception_handler
EXCEPTION_HANDLER 2, naive_exception_handler
EXCEPTION_HANDLER 3, naive_exception_handler
EXCEPTION_HANDLER 4, naive_exception_handler
EXCEPTION_HANDLER 5, naive_exception_handler
EXCEPTION_HANDLER 6, naive_exception_handler
EXCEPTION_HANDLER 7, naive_exception_handler
EXCEPTION_HANDLER 8, naive_exception_handler
EXCEPTION_HANDLER 9, naive_exception_handler
EXCEPTION_HANDLER 10, naive_exception_handler
EXCEPTION_HANDLER 11, naive_exception_handler
EXCEPTION_HANDLER 12, naive_exception_handler
EXCEPTION_HANDLER 13, naive_exception_handler
EXCEPTION_HANDLER 14, naive_exception_handler
EXCEPTION_HANDLER 15, naive_exception_handler
EXCEPTION_HANDLER 16, naive_exception_handler
EXCEPTION_HANDLER 17, naive_exception_handler
EXCEPTION_HANDLER 18, naive_exception_handler
EXCEPTION_HANDLER 19, naive_exception_handler

# Interrupt entries
.macro DEF_INTERRUPT_ENTRY irq, func
.globl interrupt_entry_\irq
interrupt_entry_\irq:
    SAVE_ALL_REGS
    call \func
    pushl $\irq  # push irq number as argument
    call send_eoi
    addl $4, %esp 
    RESTORE_ALL_REGS
    iret  # return from interrupt 
.endm

DEF_INTERRUPT_ENTRY 1, keyboard_interrupt_handler
DEF_INTERRUPT_ENTRY 8, rtc_interrupt_handler

# system call handler
EXCEPTION_HANDLER 128, naive_system_call_handler














