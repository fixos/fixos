CC=sh3eb-elf-gcc
CCFLAGS=-g -Wall -m3 -mb -Os $(INCLUDE_PATH) -fno-builtin -include config.h

LDFLAGS=-T"$(LDSCRIPT)" -nostdlib

OBJCOPY=sh3eb-elf-objcopy
OBJCOPY_FLAGS=-S

INCLUDE_PATH=-I.
LDSCRIPT=fixos.ld

KERNELNAME=fixos



all: config.mk $(KERNELNAME) user bootloader


-include config.mk

# include each subdirectory makefile part (avoid the first Makefile parse)
ifeq ($(__CONFIG__),y)
include arch/subdir.mk
include device/subdir.mk
include gcc_fix/subdir.mk
include fs/subdir.mk
include loader/subdir.mk
include sys/subdir.mk
include utils/subdir.mk
endif


# top level sources

C_SRC+= \
	init.c tests.c

ASM_SRC+= \
	initialize.s 



OBJ:=$(filter %.o, $(C_SRC:.c=.o) $(ASM_SRC:.S=.o) $(ASM_SRC:.s=.o))
DEPS:=$(C_SRC:.c=.d)


$(KERNELNAME): $(OBJ)
	$(CC) $(LDFLAGS) -o $@.big $^
	$(OBJCOPY) $(OBJCOPY_FLAGS) $@.big $@



# auto-generated dependencies (assume CC is compatible with gcc -M family commands)
# (may use -MG if generated headers are used?)
-include $(DEPS)

%.d:
	@touch $@


%.o: %.c
	$(CC) -c $(CCFLAGS) -o $@ $<
	$(CC) $(CCFLAGS) -M -MF"$(<:.c=.d)" -MP -MT"$@" -MT"$(<:.c=.d)" "$<"

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
	rm -f $(OBJ) $(DEPS) config.mk
	$(MAKE) -C user/ clean
	$(MAKE) -C bootloader/ clean

distclean: clean
	rm -f $(G1A)
	$(MAKE) -C user/ distclean
	$(MAKE) -C bootloader/ distclean

# yes, that seems weird, but ensures order (even if invoked with -jX)
re:
	$(MAKE) -C ./ distclean
	$(MAKE) -C ./ all



# autogenerate configuration file for makefiles from config.h

CONF_REGEX=^\#define[[:blank:]]\+\([_[:alnum:]]\+\)[[:blank:]]\+\([_[:alnum:]]\+\)\+.*

config.mk: config.h
	@echo Creating makefile configuration from '$<'
	@grep '$(CONF_REGEX)' $< | sed 's/$(CONF_REGEX)/\1=\2/' > $@

config.h:
	$(error File '$@' not found, needed for makefile configuration configuration)
