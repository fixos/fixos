# makefile part for arch/sh/ sub-directory


C_SRC_L:= \
	physical_memory.c mmu.c virtual_memory.c \
	interrupt.c exception.c \
	memory/c_s29jl032h.c \
	kdelay.c\
	modules/sdhi.c \
	modules/usb.c \
	process.c rtc.c timer.c time.c \
	freq.c \
	signal.c


ASM_SRC_L:= \
	interrupt_asm.s \
	exception_pre.s tlbmiss_pre.s interrupt_pre.s \
	scheduler.S \
	signal_trampoline.S


C_SRC:=$(C_SRC) $(addprefix arch/sh/, $(C_SRC_L))
ASM_SRC:=$(ASM_SRC) $(addprefix arch/sh/, $(ASM_SRC_L))
