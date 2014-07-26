#include <fs/file_operations.h>
#include <fs/file.h>
#include <device/device.h>
#include <sys/memory.h>
#include <utils/strutils.h>
#include "generic_mono.h"
#include "display.h"

#include <interface/display.h>
#include <interface/errno.h>

#include <sys/process.h>


static unsigned char *_display_vram; 

const struct device _display_device = {
	.name = "display",
	.init = display_init,
	.open = display_open
};


static const struct file_operations _fop_display = {
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

	return -ENXIO;
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
	return -EINVAL;
}

static int _ioctl_mapvram(void **virt_vram) {
	if(virt_vram != NULL) {
		// the goal is to do a mmap-like, and to asign _display_vram buffer
		// to a process user-space area
		// FIXME will work only in case of 1024 bytes buffer in a single page
		union pm_page page;
		process_t *cur;

		cur = process_get_current();

		page.private.ppn = PM_PHYSICAL_PAGE(_display_vram);
		page.private.flags = MEM_PAGE_PRIVATE | MEM_PAGE_VALID | MEM_PAGE_CACHED;
		mem_insert_page(& cur->dir_list , &page, (void*)0x18000000);

		*virt_vram = (void*)0x18000000;
		return 0;

	}
	return -EINVAL;
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
			retval = -EINVAL;
		}
		break;

	case DISPCTL_DISPLAY:
		if((int)(filep->private_data) != 0) {
			// FIXME There is a typical cause of "cache/MMU synonym problem"
			// for SH3 here, this will be a serious problem with shared memory!
			// for now, the fix is really dirty (we use the same virtual
			// address than the one used in userspace...)
			// FIXME!
			//disp_mono_copy_to_dd(_display_vram);
			disp_mono_copy_to_dd((void*)0x18000000);
		}
		else {
			retval = -EINVAL;
		}
		break;

	case DISPCTL_MAPVRAM:
		retval = _ioctl_mapvram(data);
		break;

	default:
		retval = -EINVAL;
	}

	return retval;
}
