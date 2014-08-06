# makefile part for sys/ sub-directory

C_SRC_L:= \
	process.c files.c scheduler.c \
	time.c stimer.c\
	mutex.c \
	signal.c \
	syscall.c \
	cmdline.c \
	console.c \
	memory.c \
	kdebug.c \
	sysctl.c \
	waitqueue.c \
	tty.c


CURDIR:=sys
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))

