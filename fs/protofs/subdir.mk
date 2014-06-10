# makefile part for fs/protofs/ sub-directory

C_SRC_L:= \
	file_system.c


CURDIR:=fs/protofs
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))

