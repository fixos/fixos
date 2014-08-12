include global.mk

INCLUDE_PATH:=-I. -Iinterface
CFLAGS:=$(INCLUDE_PATH) -include config.h $(CFLAGS)
#FIXME need -nostdinc

LDSCRIPT:=fixos.ld
LDFLAGS:=-T"$(LDSCRIPT)" $(LDFLAGS)


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

ifeq ($(CONFIG_DEBUG_STACK),y)
CFLAGS+=
#-funwind-tables -fno-omit-frame-pointer
# frame pointer with GCC are not useful for sh3 stack analysis
# (local variables between @r14 and previous frame pointer...)
# TODO implement limited DWARF engine to have more accurate analysis
$(warning, Config option "CONFIG_DEBUG_STACK" is not implemented (no effect))
endif


# top level sources

C_SRC+= \
	init.c tests.c

ASM_SRC+= \
	initialize.s 



OBJ:=$(filter %.o, $(C_SRC:.c=.o) $(ASM_SRC:.S=.o) $(ASM_SRC:.s=.o))
DEPS:=$(C_SRC:.c=.d)

$(KERNELNAME).big: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(KERNELNAME).debug: $(OBJ) $(DEBUG_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(KERNELNAME): $(KERNELNAME).debug
	$(OBJCOPY) -S $< $@


# command line used to generate the "text symbols" informations

# auto-generated dependencies (assume CC is compatible with gcc -M family commands)
# (may use -MG if generated headers are used?)
-include $(DEPS)

%.d:
	@touch $@


%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
	$(CC) $(CFLAGS) -M -MF"$(<:.c=.d)" -MP -MT"$@" -MT"$(<:.c=.d)" "$<"

%.o: %.s
	$(CC) -x assembler-with-cpp -c $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) -x assembler-with-cpp -c $(CFLAGS) -o $@ $<


# Userland stuff sub-makefile
user:
	$(MAKE) -C user/ all

# Bootloader makefile
bootloader:
	$(MAKE) -C bootloader/ all


.PHONY: clean distclean re all user bootloader

clean:
	rm -f $(OBJ) $(DEPS) $(KERNELNAME).big $(KERNELNAME).debug $(DEBUG_OBJ)  config.mk
	$(MAKE) -C user/ clean
	$(MAKE) -C bootloader/ clean

distclean: clean
	rm -f $(KERNELNAME)
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

# as config.h is included in every compiled file, this is a big dependency
$(C_SRC) $(ASM_SRC): config.h


