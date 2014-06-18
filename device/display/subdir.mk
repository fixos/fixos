# makefile part for device/display/ sub-directory

ifeq ($(CONFIG_PLATFORM),fx9860)
include device/display/T6K11/subdir.mk
endif

C_SRC_L:= \
	display.c


CURDIR:=device/display
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))
