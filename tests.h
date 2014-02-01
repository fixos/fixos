#ifndef _FIXOS_TESTS_H
#define _FIXOS_TESTS_H

/**
 * Lot of misc tests for debuging and usage example.
 */

#define DBG_WAIT  while(is_key_down(K_EXE)); \
	while(!is_key_down(K_EXE))


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
#endif //_FIXOS_TESTS_H
