# makefile part for loader/elfloader/ sub-directory

C_SRC_L:= \
	loader.c


CURDIR:=loader/elfloader
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))

