CC=sh3eb-elf-gcc
CCFLAGS=-g -Wall -m3 -mb -Os $(INCLUDE_PATH) -fno-builtin

LDFLAGS=-T"$(LDSCRIPT)" -nostdlib

OBJCOPY=sh3eb-elf-objcopy
ELFTOBIN_OBJCOPY_FLAGS=-R .comment -R .bss -O binary

INCLUDE_PATH=-I..
LDSCRIPT=bootloader.ld

G1A_WRAPPER=c_g1awrapper
G1A_ICON=../icon.bmp


BASENAME=bootldr
BINARY=$(BASENAME).bin
ELF=$(BASENAME).elf
G1A=$(BASENAME).g1a

C_SRC=bootloader.c smem_file.c config_parser.c \
	  ../fs/casio_smemfs/smemfs_primitives_ng.c
ASM_SRC=bootloader_pre.s casio_syscalls.S \
		../utils/sh/memcpy.S ../utils/sh/strcmp.S

TMPSTUB:=$(ASM_SRC:.S=.o)
OBJ=$(C_SRC:.c=.o) $(TMPSTUB:.s=.o)


all: $(G1A)

$(G1A): $(BINARY)
	$(G1A_WRAPPER) $< -o $@ -i $(G1A_ICON) -n $(BASENAME)

$(BINARY): $(ELF)
	$(OBJCOPY) $(ELFTOBIN_OBJCOPY_FLAGS) $< $@

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
	rm -f $(OBJ) $(ELF) $(BIN)

distclean: clean
	rm -f $(EXEC)

re: distclean all

