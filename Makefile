CC=sh3eb-elf-gcc
CCFLAGS=-g -Wall -m3 -mb -Os $(INCLUDE_PATH) -fno-builtin

LDFLAGS=-T"$(LDSCRIPT)" -nostdlib

OBJCOPY=sh3eb-elf-objcopy
ELFTOBIN_OBJCOPY_FLAGS=-R .comment -R .bss -O binary

BOOTLOADER_OBJCOPY_FLAGS=--rename-section .rodata.str1.4=.bootloader.rodata \
						 --rename-section .text=.bootloader

INCLUDE_PATH=-I.
LDSCRIPT=fixos.ld

G1A_WRAPPER=c_g1awrapper
G1A_ICON=icon.bmp

BASENAME=fixos
BINARY=$(BASENAME).bin
ELF=$(BASENAME).elf
G1A=$(BASENAME).g1a

C_SRC=loader/ramloader/loader.c loader/elfloader/loader.c \
	  fs/vfs_cache.c fs/vfs_op.c fs/vfs.c fs/vfs_file.c \
	  fs/protofs/file_system.c \
	  fs/casio_smemfs/smemfs_primitives_ng.c fs/casio_smemfs/file_system.c fs/casio_smemfs/file.c \
	  utils/strconv.c utils/log.c \
	  arch/sh/physical_memory.c arch/sh/mmu.c arch/sh/virtual_memory.c arch/sh/interrupt.c \
	  arch/sh/exception.c arch/sh/memory/c_s29jl032h.c arch/sh/kdelay.c arch/sh/modules/sdhi.c \
	  sys/process.c sys/files.c \
	  device/device_registering.c \
	  device/keyboard/iskeydown.c \
	  device/terminal/fx9860/early_term.c device/terminal/fx9860/print_primitives.c device/terminal/fx9860/terminal.c \
	  device/display/T6K11/T6K11.c \
	  syscalls/arch/syscall.c \
	  init.c tests.c

ASM_SRC=utils/sh/strcmp.S arch/sh/interrupt_asm.s gcc_fix/udivsi3_i4i.S initialize.s utils/sh/memcpy.S utils/sh/memset.S utils/sh/strcpy.S utils/sh/strlen.S arch/sh/exception_pre.s arch/sh/tlbmiss_pre.s arch/sh/interrupt_pre.s \
		device/display/T6K11/drawall.s device/display/T6K11/setpixel.s 


# Specific rules are applied to bootloader object files (no usage of .data/.bss
# is allowed, and .rodata must be renamed to be mapped near the bootloader code)
BOOTLOADER_C_SRC=bootloader/bootloader.c bootloader/minimalist_smemfs.c

BOOTLOADER_ASM_SRC=bootloader/bootloader_pre.s bootloader/bootloader_memcpy.S



BOOTLOADER_TMPSTUB:=$(BOOTLOADER_ASM_SRC:.s=.o)
BOOTLOADER_OBJ=$(BOOTLOADER_C_SRC:.c=.o) $(BOOTLOADER_TMPSTUB:.S=.o)
BOOTLOADER_OBJ_MODIFIED=$(BOOTLOADER_OBJ:.o=_modified.o)

TMPSTUB:=$(ASM_SRC:.s=.o)
OBJ=$(C_SRC:.c=.o) $(TMPSTUB:.S=.o) $(BOOTLOADER_OBJ_MODIFIED)




all: $(G1A) user

$(G1A): $(BINARY)
	$(G1A_WRAPPER) $< -o $@ -i $(G1A_ICON) -n $(BASENAME)

$(BINARY): $(ELF)
	$(OBJCOPY) $(ELFTOBIN_OBJCOPY_FLAGS) $< $@

$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(BOOTLOADER_OBJ_MODIFIED): %_modified.o: %.o
	$(OBJCOPY) $(BOOTLOADER_OBJCOPY_FLAGS) $< $@

%.o: %.c
	$(CC) -c $(CCFLAGS) -o $@ $<

%.o: %.s
	$(CC) -c $(CCFLAGS) -o $@ $<

%.o: %.S
	$(CC) -c $(CCFLAGS) -o $@ $<


# Userland stuff sub-makefile
user:
	$(MAKE) -C user/ all

.PHONY: clean distclean re all user

clean:
	rm -f $(OBJ) $(BOOTLOADER_OBJ) $(ELF) $(BINARY)
	$(MAKE) -C user/ clean

distclean: clean
	rm -f $(G1A)
	$(MAKE) -C user/ distclean

re: distclean all
