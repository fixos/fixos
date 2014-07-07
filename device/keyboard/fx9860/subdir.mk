# makefile part for device/keyboard/fx9860 sub-directory

C_SRC_L:= \
	keymatrix.c keyboard.c \
	keyboard_device.c


CURDIR:=device/keyboard/fx9860
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))
