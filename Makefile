CC=sh3eb-elf-gcc
CCFLAGS=-g -Wall -m3 -mb -Os $(INCLUDE_PATH) -fno-builtin

LDFLAGS=-T"$(LDSCRIPT)" -nostdlib

OBJCOPY=sh3eb-elf-objcopy
OBJCOPY_FLAGS=-S

INCLUDE_PATH=-I.
LDSCRIPT=fixos.ld

KERNELNAME=fixos

C_SRC=loader/ramloader/loader.c loader/elfloader/loader.c \
	  fs/vfs_cache.c fs/vfs_op.c fs/vfs.c fs/vfs_file.c \
	  fs/protofs/file_system.c \
	  fs/casio_smemfs/smemfs_primitives_ng.c fs/casio_smemfs/file_system.c fs/casio_smemfs/file.c \
	  utils/strconv.c utils/log.c utils/cyclic_fifo.c \
	  arch/sh/physical_memory.c arch/sh/mmu.c arch/sh/virtual_memory.c arch/sh/interrupt.c \
	  arch/sh/exception.c arch/sh/memory/c_s29jl032h.c arch/sh/kdelay.c arch/sh/modules/sdhi.c \
	  sys/process.c sys/files.c sys/scheduler.c \
	  device/device_registering.c \
	  device/keyboard/fx9860/keymatrix.c device/keyboard/fx9860/keyboard.c \
	  device/terminal/fx9860/early_term.c device/terminal/fx9860/print_primitives.c device/terminal/fx9860/terminal.c \
	  device/display/T6K11/T6K11.c \
	  device/usb/cdc_acm/cdc_acm.c device/usb/cdc_acm/acm_device.c \
	  arch/sh/modules/usb.c \
	  arch/sh/process.c arch/sh/rtc.c arch/sh/timer.c arch/sh/time.c\
	  arch/sh/freq.c \
	  syscalls/arch/syscall.c \
	  sys/time.c \
	  init.c tests.c

ASM_SRC=utils/sh/strcmp.S arch/sh/interrupt_asm.s gcc_fix/udivsi3_i4i.S initialize.s utils/sh/memcpy.S utils/sh/memset.S utils/sh/strcpy.S utils/sh/strlen.S arch/sh/exception_pre.s arch/sh/tlbmiss_pre.s arch/sh/interrupt_pre.s \
	  arch/sh/scheduler.S \
	  device/display/T6K11/drawall.s device/display/T6K11/setpixel.s 


TMPSTUB:=$(ASM_SRC:.s=.o)
OBJ=$(C_SRC:.c=.o) $(TMPSTUB:.S=.o)


all: $(KERNELNAME) user bootloader

$(KERNELNAME): $(OBJ)
	$(CC) $(LDFLAGS) -o $@.big $^
	$(OBJCOPY) $(OBJCOPY_FLAGS) $@.big $@


%.o: %.c
	$(CC) -c $(CCFLAGS) -o $@ $<

%.o: %.s
	$(CC) -c $(CCFLAGS) -o $@ $<

%.o: %.S
	$(CC) -c $(CCFLAGS) -o $@ $<


# Userland stuff sub-makefile
user:
	$(MAKE) -C user/ all

# Bootloader makefile
bootloader:
	$(MAKE) -C bootloader/ all


.PHONY: clean distclean re all user bootloader

clean:
	rm -f $(OBJ) $(ELF) $(BINARY)
	$(MAKE) -C user/ clean
	$(MAKE) -C bootloader/ clean

distclean: clean
	rm -f $(G1A)
	$(MAKE) -C user/ distclean
	$(MAKE) -C bootloader/ distclean

re: distclean all
