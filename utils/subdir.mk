# makefile part for utils/ sub-directory

C_SRC_L:= \
	strconv.c \
	log.c \
	cyclic_fifo.c pool_alloc.c


ifeq ($(CONFIG_ARCH),sh3)
ASM_SRC_L:= \
	sh/strcmp.S sh/memcpy.S sh/memset.S sh/strcpy.S sh/strlen.S
endif


CURDIR:=utils
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))
ASM_SRC:=$(ASM_SRC) $(addprefix $(CURDIR)/, $(ASM_SRC_L))

