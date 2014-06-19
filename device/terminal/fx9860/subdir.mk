# makefile part for device/terminal/fx9860/ sub-directory

C_SRC_L:= \
	early_term.c print_primitives.c terminal.c text_display.c
	
	
CURDIR:=device/terminal/fx9860
C_SRC:=$(C_SRC) $(addprefix $(CURDIR)/, $(C_SRC_L))

