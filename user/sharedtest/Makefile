include ../../global.mk


# need to export some tools for shared-lib scripts
export READELF
export OBJCOPY
export NM

# path for shared library helper scripts
SHARED3_PATH:=$(shell pwd)/tools
PATH:=$(PATH):$(SHARED3_PATH)

MK_STUBS:=$(SHARED3_PATH)/make_shared_stubs.sh
STRIP_UNSHARED:=$(SHARED3_PATH)/strip_unshared.sh

INCLUDE_PATH:=-I.. -I../../interface
CFLAGS:=-fvisibility=hidden -fpic $(INCLUDE_PATH) $(CFLAGS)

LDSCRIPT:=userland-so.ld
LDFLAGS:=-T"$(LDSCRIPT)" -shared $(LDFLAGS)


LIBNAME:=test

LIB:=lib$(LIBNAME).so
LIBSTUB:=lib$(LIBNAME)-shared.a
LIB_NOSTRIP:=lib$(LIBNAME)-nostrip.so


C_SRC:=test.c ../lib/syscalls.c
	
ASM_SRC:=

OBJ:=$(filter %.o, $(C_SRC:.c=-pic.o) $(ASM_SRC:.S=.o) $(ASM_SRC:.s=.o)) 


.PHONY: clean distclean re all


all: $(LIBSTUB) $(LIB)

$(LIB_NOSTRIP): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(LIB): $(LIB_NOSTRIP)
	"$(STRIP_UNSHARED)" $< $@ 
#	$(OBJCOPY) -S $@ $<

# not really clean, but easier than an include containing dynamicaly created
# stub rules
$(LIBSTUB): $(LIB) stubs
	for stub in `ls stubs/*.s` ; do \
		$(CC) -c $(CFLAGS) "$$stub" -o "$$stub".o ; \
	done; 
	$(AR) rcs $@ `ls stubs/*.o`

stubs: $(LIB)
	rm -rf stubs/
	"$(MK_STUBS)" $< $< stubs
	

%-pic.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.s
	$(CC) -x assembler-with-cpp -c $(CFLAGS) -o $@ $<

%.o: %.S
	$(CC) -x assembler-with-cpp -c $(CFLAGS) -o $@ $<



clean:
	rm -f $(OBJ) $(LIB_NOSTRIP)
	rm -rf stubs/

distclean: clean
	rm -f $(LIB) $(LIBSTUB)

re: distclean all
