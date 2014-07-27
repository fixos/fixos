# makefile part for fs/ sub-directory

include fs/casio_smemfs/subdir.mk
include fs/protofs/subdir.mk


C_SRC_L:= \
	vfs_cache.c vfs_op.c vfs.c vfs_file.c pipe.c \
	vfs_directory.c


CURDIR:=fs
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))
