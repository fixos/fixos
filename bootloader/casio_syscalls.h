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

#ifndef _BOOTLOADER_CASIO_SYSCALLS_H
#define _BOOTLOADER_CASIO_SYSCALLS_H

/**
 * This file defines Casio's OS 'syscalls', used by the bootloader before any
 * part of the kernel is load.
 * These functions are used to display and get user input in boot screen
 * (they will work fine until any dirty thing is done, like erasing all the
 * RAM used by Casio's OS...)
 *
 * Do not forget it's like a miracle to use system-provided functions, and in
 * the same time use physical memory, hard-coded addresses, to store .bss,
 * .data and stack, so if a problem occurs in bootloader step, this is the
 * first thing we have to check (no overwrite of OS's data).
 */


// print msg at (x;y), normal if type == 0, reversed if type == 1
// size of the font used is 6*8 (21*8 characters)
void casio_PrintXY(int x, int y, const char *msg, int type);

// same as casio_PrintXY, but font is 4*6 (32*10 characters)
void casio_PrintMini(int x, int y, const char *msg, int type);

// clear VRAM
void casio_Bdisp_AllClr_VRAM();


// put VRAM on display driver
void casio_Bdisp_PutDisp_DD();


// wait for user to press a key and set its code in keycode
int casio_GetKey(unsigned int *keycode);


// only some keycodes are used :
#define KEY_CTRL_EXE        30004
#define KEY_CTRL_SHIFT      30006
#define KEY_CTRL_UP         30018
#define KEY_CTRL_DOWN       30023
#define KEY_CTRL_LEFT       30020
#define KEY_CTRL_RIGHT      30021


#endif //_BOOTLOADER_CASIO_SYSCALLS_H
