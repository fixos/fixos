include ../global.mk


INCLUDE_PATH:=-I.. -I../interface
CFLAGS:=$(INCLUDE_PATH) $(CFLAGS)

LDSCRIPT:=bootloader.ld
LDFLAGS:=-T"$(LDSCRIPT)" $(LDFLAGS)

ELFTOBIN_OBJCOPY_FLAGS:=-R .comment -R .bss -O binary


G1A_ICON:=../icon.bmp

BASENAME:=bootldr
BINARY:=$(BASENAME).bin
ELF:=$(BASENAME).elf
G1A:=$(BASENAME).g1a

C_SRC:=bootloader.c smem_file.c config_parser.c elf_loader.c \
	  ../fs/casio_smemfs/smemfs_primitives_ng.c
ASM_SRC:=bootloader_pre.s casio_syscalls.S \
		../utils/sh/memcpy.S ../utils/sh/strcmp.S ../utils/sh/memset.S


OBJ:=$(filter %.o, $(C_SRC:.c=.o) $(ASM_SRC:.S=.o) $(ASM_SRC:.s=.o))


all: $(G1A)

$(G1A): $(BINARY)
	$(G1A_WRAPPER) $< -o $@ -i $(G1A_ICON) -n $(BASENAME)

$(BINARY): $(ELF)
	$(OBJCOPY) $(ELFTOBIN_OBJCOPY_FLAGS) $< $@

$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^


%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.s
	$(CC) -x assembler-with-cpp -c $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) -x assembler-with-cpp -c $(CFLAGS) -o $@ $<


.PHONY: clean distclean re all

clean:
	rm -f $(OBJ) $(ELF) $(BIN)

distclean: clean
	rm -f $(EXEC)

re: distclean all

