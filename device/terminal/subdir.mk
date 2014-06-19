# makefile part for device/terminal sub-directory

ifeq ($(CONFIG_PLATFORM),fx9860)
include device/terminal/fx9860/subdir.mk
endif

C_SRC_L:= \
	virtual_term.c


CURDIR:=device/terminal
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))
