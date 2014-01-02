CC=sh3eb-elf-gcc
CCFLAGS=-g -Wall -m3 -mb -Os $(INCLUDE_PATH) -fno-builtin

LDFLAGS=-T"$(LDSCRIPT)" -nostdlib

OBJCOPY=sh3eb-elf-objcopy
OBJCOPY_FLAGS=-S

INCLUDE_PATH=-I.
LDSCRIPT=userland.ld

EXEC=test.elf
ENTRY_POINT=_usertest_main

C_SRC=first_test_elf.c lib/syscalls.c
OBJ=$(C_SRC:.c=.o)


all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(LDFLAGS) -e $(ENTRY_POINT) -o $@.big $^
	$(OBJCOPY) -S $@.big $@
	rm $@.big

%.o: %.c
	$(CC) -c $(CCFLAGS) -o $@ $<

%.o: %.s
	$(CC) -c $(CCFLAGS) -o $@ $<

%.o: %.S
	$(CC) -c $(CCFLAGS) -o $@ $<


.PHONY: clean distclean re all

clean:
	rm -f $(OBJ)

distclean: clean
	rm -f $(EXEC)

re: distclean all
