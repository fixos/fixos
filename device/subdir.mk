# makefile part for device/ sub-directory

include device/display/subdir.mk 
include device/keyboard/subdir.mk 
include device/terminal/subdir.mk 
include device/usb/subdir.mk 


C_SRC_L:= \
	device_registering.c


CURDIR:=device
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))
