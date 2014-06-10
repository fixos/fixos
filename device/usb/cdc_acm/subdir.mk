# makefile part for device/usb/cdc_acm/ sub-directory

C_SRC_L:= \
	cdc_acm.c acm_device.c


CURDIR:=device/usb/cdc_acm
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))

