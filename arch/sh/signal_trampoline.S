!
! Contains the signal trampoline code, which allow to return from user space
! signal handler to user space previous context, using the syscall sigreturn()
! This small piece of code is designed to be copied into the user stack, where
! it will be executed.
!

#include <interface/fixos/syscalls.h>

	.section ".text"
	
	.global _sig_trampoline_begin
	.global _sig_trampoline_end

	.align 4
_sig_trampoline_begin:
	trapa #SYSCALL_SIGRETURN
	nop
_sig_trampoline_end:
	nop

