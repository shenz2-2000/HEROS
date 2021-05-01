#define ASM_SIGNAL  1

#include "signal_sys_call.h"
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

     # save callee-saved register, for C convention
     pushl %ebp
     movl  %esp, %ebp

     pushl %edi
     pushl %esi
     pushl %ebx


     # -----------------------Prepare for calling user_level handler--------------------

     # store the parameters
     movl 8(%ebp), %edi
     movl %edi, storage_cur_signal_ptr
     movl 12(%ebp), %edi
     movl %edi, storage_handler_ptr
     movl 16(%ebp), %edi
     movl %edi, storage_hw_ptr

     # get user esp and ebp
     movl storage_hw_ptr, %edi

     movl 20(%edi), %esi
     movl %esi, user_ebp
     movl 60(%edi), %esi
     movl %esi, user_esp

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
     movl storage_hw_ptr, %edi
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
     movl storage_hw_ptr, %edi
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

      pushl %edx
      pushl %ecx
      pushl %eax

      call restore_signal

      popl  %eax
      popl  %ecx
      popl  %edx

      movl kernel_hw_context_ptr, %esi

      movl 24(%esi), %eax


      ret
