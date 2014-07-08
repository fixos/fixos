include ../global.mk


INCLUDE_PATH:=-I. -I../interface
CFLAGS:=$(INCLUDE_PATH) $(CFLAGS) -Wno-unused-function

LDSCRIPT:=userland.ld
LDFLAGS:=-T"$(LDSCRIPT)" $(LDFLAGS)

LDLIBS:=-Lsharedtest -ltest-shared


EXEC:=test.elf
ENTRY_POINT:=_usertest_main

C_SRC:=first_test_elf.c lib/syscalls.c dynld.c
	
ASM_SRC:=dynld_solvename_call.s
#../gcc_fix/udivsi3_i4i.S

OBJ:=$(filter %.o, $(C_SRC:.c=.o) $(ASM_SRC:.S=.o) $(ASM_SRC:.s=.o)) 



all: $(EXEC)

$(EXEC): sharedlib $(OBJ)
	$(CC) $(LDFLAGS) -e $(ENTRY_POINT) -o $@.big $(OBJ) $(LDLIBS)
	$(OBJCOPY) -S $@.big $@

sharedlib:
	$(MAKE) -C sharedtest/ all 

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.s
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) -c $(CFLAGS) -o $@ $<


.PHONY: clean distclean re all sharedlib

clean:
	rm -f $(OBJ)
	rm -f $(EXEC).big
	$(MAKE) -C sharedtest/ clean

distclean: clean
	rm -f $(EXEC)
	$(MAKE) -C sharedtest/ distclean

re: distclean all
