# makefile part for loader/ramloader/ sub-directory

C_SRC_L:= \
	loader.c


CURDIR:=loader/ramloader
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))
