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

#ifndef _FIXOS_TESTS_H
#define _FIXOS_TESTS_H

/**
 * Lot of misc tests for debuging and usage example.
 */

#include <device/keyboard/fx9860/keymatrix.h>
#include <device/keyboard/fx9860/matrix_codes.h>

#define DBG_WAIT  while(hwkbd_real_keydown(K_EXE)); \
	while(!hwkbd_real_keydown(K_EXE))


void test();

void print_content(void *addr, int size);

/**
 * Function to help debugging fs's and vfs : print the tree
 * of ALL the mounted file system (wondereful)
 */
void ls_tree();


/**
 * Tests for SD Card related functions.
 */
void test_sdcard();


/**
 * Tests for sleep-like kernel functions.
 */
void test_sleep_funcs();


/**
 * Tests for read/write on EEPROM chip.
 * WARNING: These functionalities are risky for now, do not use them if you
 * aren't SURE about what you're doing!
 */
void test_eeprom();


/**
 * Test user process creation and execution.
 * (never return if the process was created correctly)
 */
void test_process();


/**
 * Test VFS functionalities.
 */
void test_vfs();


/**
 * Test Virtual Memory functionalities.
 */
void test_virtual_mem();


/**
 * Test keyboard interrupts.
 */
void test_keyboard_int();


/**
 * Test time-related things (TMU, RTC...)
 */
void test_time();


/**
 * Test key press handling using key matrix (ports A and B/M)
 */
void test_keymatrix();


/**
 * Test 'high level' keyboard handling.
 */
void test_keyboard();

#endif //_FIXOS_TESTS_H
