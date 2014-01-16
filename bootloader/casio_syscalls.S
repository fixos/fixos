
#define CASIO_SYS_ALLCLR_VRAM	0x143
#define CASIO_SYS_PUTDISP_DD	0x028
#define CASIO_SYS_GETKEY		0x90F
#define CASIO_SYS_PRINTXY		0x150
#define CASIO_SYS_PRINTMINI		0xC4F


#define SYSCALL(name,num) \
	.global	name ; \
	.type name, @function ; \
	.align 2 ; \
name: \
	mov.l SyscallAddress, r1 ; \
	mov.l name ## _id, r0 ; \
	jmp @r1 ; \
	nop ; \
	.align 4 ;\
name ## _id: \
	.long num ; \


	.section ".text"

SYSCALL(_casio_Bdisp_AllClr_VRAM, CASIO_SYS_ALLCLR_VRAM)


SYSCALL(_casio_PrintXY, CASIO_SYS_PRINTXY)

SYSCALL(_casio_PrintMini, CASIO_SYS_PRINTMINI)


SYSCALL(_casio_Bdisp_PutDisp_DD, CASIO_SYS_PUTDISP_DD)


SYSCALL(_casio_GetKey, CASIO_SYS_GETKEY)
	


	.align 4
SyscallAddress:
	.long 0x80010070

