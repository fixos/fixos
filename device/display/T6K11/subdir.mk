# makefile part for device/display/T6K11/ sub-directory

C_SRC_L:= \
	T6K11.c

ASM_SRC_L:= \
	drawall.s setpixel.s
	


CURDIR:=device/display/T6K11
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))
ASM_SRC:=$(ASM_SRC) $(addprefix $(CURDIR)/, $(ASM_SRC_L))
