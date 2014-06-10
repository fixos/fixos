# makefile part for device/keyboard/ sub-directory

ifeq ($(CONFIG_PLATFORM),fx9860)
include device/keyboard/fx9860/subdir.mk
endif
