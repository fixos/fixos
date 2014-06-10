# makefile part for device/terminal sub-directory

ifeq ($(CONFIG_PLATFORM),fx9860)
include device/terminal/fx9860/subdir.mk
endif
