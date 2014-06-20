#include <fs/file_operations.h>
#include <fs/file.h>
#include <device/device.h>
#include <sys/memory.h>
#include <utils/strutils.h>
#include "generic_mono.h"
#include "display.h"

#include <interface/display.h>

#include <arch/sh/virtual_memory.h>
#include <sys/process.h>


static unsigned char *_display_vram; 

struct device _display_device = {
	.name = "display",
	.init = display_init,
	.open = display_open
};


static struct file_operations _fop_display = {
	.release = display_release,
	.write = NULL,
	.read = NULL,
	.ioctl = display_ioctl
};


static int _activated = 0;


void display_init() {
	// initialize VRAM and other things
	// TODO should not assume screen VRAM size is 1024 bytes
	
	_display_vram = mem_pm_get_free_page(MEM_PM_CACHED);

	memset(_display_vram, 0, 1024);
}


int display_open(uint16 minor, struct file *filep) {
	// consider this function may be called many times, and only for minor 1
	
	if(minor == DISPLAY_DEFAULT_MINOR) {
		filep->op = &_fop_display;
		filep->private_data = 0;
		return 0;
	}

	return -1;
}


int display_release(struct file *filep) {
	// release display if it was activated by this file
	if(filep->private_data != 0) {
		_activated = 0;
	}
	
	return 0;
}


static int _ioctl_info(struct display_info *infos) {
	if(infos != NULL) {
		infos->bpp = 1;
		infos->flags = 0;
		infos->format = DISP_FORMAT_MONO;
		infos->height = disp_mono_height();
		infos->width = disp_mono_width();
		infos->vram_size = 1024;

		return 0;
	}
	return -1;
}

static int _ioctl_mapvram(void **virt_vram) {
	if(virt_vram != NULL) {
		// the goal is to do a mmap-like, and to asign _display_vram buffer
		// to a process user-space area
		// FIXME will work only in case of 1024 bytes buffer in a single page
		struct _vm_page vpage;
		process_t *cur;

		mem_vm_prepare_page(&vpage, _display_vram, (void*)0x18000000, MEM_VM_CACHED);
		cur = process_get_current();
		vm_add_entry(& cur->vm, &vpage);

		*virt_vram = (void*)0x18000000;
		return 0;

	}
	return -1;
}


int display_ioctl(struct file *filep, int cmd, void *data) {
	int retval = 0;

	switch(cmd) {
	case DISPCTL_INFO:
		retval = _ioctl_info(data);
		break;

	case DISPCTL_SETMODE:
		if((int)data == DISPMODE_ACTIVATE && _activated == 0) {
			filep->private_data = (void*)1;
			_activated = 1;
		}
		else if((int)data == DISPMODE_DEACTIVATE
				&& (int)(filep->private_data) == 1)
		{	
			filep->private_data = (void*)0;
			_activated = 0;
		}
		else {
			retval = -1;
		}
		break;

	case DISPCTL_DISPLAY:
		if((int)(filep->private_data) != 0) {
			disp_mono_copy_to_dd(_display_vram);
		}
		else {
			retval = -1;
		}
		break;

	case DISPCTL_MAPVRAM:
		retval = _ioctl_mapvram(data);
		break;

	default:
		retval = -1;
	}

	return retval;
}
