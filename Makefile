CC=sh3eb-elf-gcc
CCFLAGS=-Wall -m3 -mb -Os $(INCLUDE_PATH) -fno-builtin

LDFLAGS=-T"$(LDSCRIPT)" -nostdlib

OBJCOPY=sh3eb-elf-objcopy

INCLUDE_PATH=-I.
LDSCRIPT=fixos.ld

G1A_WRAPPER=c_g1awrapper
G1A_ICON=icon.bmp

BASENAME=fixos
BINARY=$(BASENAME).bin
ELF=$(BASENAME).elf
G1A=$(BASENAME).g1a

C_SRC=arch/sh/physical_memory.c arch/sh/mmu.c sys/process.c arch/sh/virtual_memory.c arch/sh/interrupt.c arch/sh/exception.c keyboard/iskeydown.c init.c display/T6K11/terminal.c sys/terminal.c

ASM_SRC=bootstrap.s arch/sh/interrupt_asm.s arch/sh/tlb_handler.S gcc_fix/udivsi3_i4i.S initialize.s display/T6K11/drawall.s display/T6K11/setpixel.s sys/sh/memcpy.S sys/sh/memset.S sys/sh/strcpy.S sys/sh/strlen.S sys/sh/strcmp.S

TMPSTUB:=$(ASM_SRC:.s=.o)
OBJ=$(C_SRC:.c=.o) $(TMPSTUB:.S=.o)



all: $(G1A)

$(G1A): $(BINARY)
	$(G1A_WRAPPER) $< -o $@ -i $(G1A_ICON) -n $(BASENAME)

$(BINARY): $(ELF)
	$(OBJCOPY) -R .comment -R .bss -O binary $< $@

$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CCFLAGS) -o $@ $<

%.o: %.s
	$(CC) -c $(CCFLAGS) -o $@ $<

%.o: %.S
	$(CC) -c $(CCFLAGS) -o $@ $<


.PHONY: clean distclean re all

clean:
	rm -f $(OBJ) $(ELF) $(BINARY)

distclean: clean
	rm -f $(G1A)

re: distclean all
