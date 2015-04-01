/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FIXOS_INTERFACE_DISPLAY_H
#define _FIXOS_INTERFACE_DISPLAY_H

/**
 * Interface for manipulation of "display" devices, allowing per-pixel access
 * to a real screen with low overhead.
 */

#include <fixos/types.h>
#include <fixos/ioctl.h>


enum display_format {
	// monochrome screen
	DISP_FORMAT_MONO
};

struct display_info {
	__kernel_size_t width;
	__kernel_size_t height;
	__kernel_size_t bpp;		// bits per pixel
	enum display_format format;
	__kernel_size_t vram_size;
	int flags;		// additional capabilities...
};


#define DISPCTL			IOCTL_NAMESPACE('D', 'I')

/**
 * Get the display informations (width/height, VRAM format, available colors...)
 */
#define DISPCTL_INFO	IOCTL_W( DISPCTL, 0x0001, struct display_info *)

/**
 * Display the current VRAM to real screen.
 */
#define DISPCTL_DISPLAY	IOCTL( DISPCTL, 0x0002)

/**
 * Try to activate or deactivate the display (if activation fail or if display
 * is not already activate, you should not modify VRAM nor try to display its
 * content).
 */
#define DISPCTL_SETMODE	IOCTL_W( DISPCTL, 0x0003, int)
#define 	DISPMODE_ACTIVATE		1
#define		DISPMODE_DEACTIVATE		2

/**
 * Temporary ioctl (before mmap() implementation), return a pointer to a user
 * space usable as VRAM.
 * Its size is vram_size bytes, from display_info struct.
 */
#define DISPCTL_MAPVRAM	IOCTL_W( DISPCTL, 0x0100, void **)

#endif //_FIXOS_INTERFACE_DISPLAY_H
