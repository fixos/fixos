# makefile part for gcc_fix/ sub-directory

ifeq ($(CONFIG_ARCH),sh3)
ASM_SRC_L:= \
	udivsi3_i4i.S movmem.S
endif
	


CURDIR:=gcc_fix
ASM_SRC:=$(ASM_SRC) $(addprefix $(CURDIR)/, $(ASM_SRC_L))
