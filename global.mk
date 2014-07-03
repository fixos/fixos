# Global options, used in each Makefile in the project
# If you have to change a tool name, or to add an option for every part of FiXos
# this is the good place.

TOOLCHAIN_PREFIX?=sh3eb-elf-

CC:=$(TOOLCHAIN_PREFIX)gcc
NM:=$(TOOLCHAIN_PREFIX)nm
OBJCOPY:=$(TOOLCHAIN_PREFIX)objcopy
READELF:=$(TOOLCHAIN_PREFIX)readelf
AR:=$(TOOLCHAIN_PREFIX)ar

G1A_WRAPPER:=c_g1awrapper


# global tool options
CFLAGS:=-g -Wall -m3 -mb -Os -fno-builtin $(CFLAGS)
LDFLAGS:=-nostdlib $(LDFLAGS)
