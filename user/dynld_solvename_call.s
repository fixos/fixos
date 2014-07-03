
	.extern _dynld_solvename
	.global dynld_solvename_call


	.section ".text"
	.align 1

	! save r4-r7 for called function, then load dynamic symbol string, try
	! to solve it, and if succeed store symbol address at PR+2 and call
	! the function
dynld_solvename_call:
	mov.l r4, @-r15
	mov.l r5, @-r15
	mov.l r6, @-r15
	mov.l r7, @-r15
	sts.l pr, @-r15
	! cache symbol (4 bytes) and symbol string
	mov.l r0, @-r15

	mov r0, r4
	add #4, r4		! should be the symbol name string
	mov.l solvename, r5
	jsr @r5
	nop

	mov.l @r15+, r1		! should be the PLT jump address
	mov.l r0, @r1

	lds.l @r15+, pr
	mov.l @r15+, r7
	mov.l @r15+, r6
	mov.l @r15+, r5
	mov.l @r15+, r4

	! jump to function
	jmp @r0
	nop

	.align 2
solvename:
	.long _dynld_solvename
