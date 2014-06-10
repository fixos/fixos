# makefile part for arch/ sub-directory


ifeq ($(CONFIG_ARCH),sh3)
include arch/sh/subdir.mk
else
$(error The architecture "$(CONFIG_ARCH)" is not supported)
endif

