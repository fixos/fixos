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
void casio_PrintXY(int x, int y, char *msg, int type);


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
