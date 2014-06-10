# makefile part for fs/casio_smemfs/ sub-directory

C_SRC_L:= \
	smemfs_primitives_ng.c file_system.c file.c


CURDIR:=fs/casio_smemfs
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))
