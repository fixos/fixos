#ifndef _DEVICE_KEYBOARD_FX9860_KEYBOARD_H
#define _DEVICE_KEYBOARD_FX9860_KEYBOARD_H

/**
 * This is the higher level of keyboard handling.
 * It use the kematrix functions to store key pressed, and translate them into
 * "key stroke" depending on keyboard status (ALPHA, SHIFT...) and various
 * parameters like the key repetition.
 * Each key stroke is signaled by calling a given callback function, and the
 * key code used is either ASCII code for standard keys or high value (>0xFF)
 * for each calc-specific key.
 */


typedef void (*kbd_key_handler)(int keycode);


/**
 * Initialize keyboard module, including keymatrix.
 * Set keymatrix callback functions to handle key state.
 */
void kbd_init();


/**
 * Return the code corresponding to the given key (from its keymatrix code)
 * using current status (SHIFT/ALPHA...).
 */
int kdb_convert_keymatrix_code(int matrixcode);



/**
 * Callback functions used by keymatrix module.
 */
void kbd_kpressed_handler(int code);
void kbd_kreleased_handler(int code);


/**
 * Set the function to call each time a key stroke happen (this can be when the
 * key is pressed, or if repetition is enable, after the first key press)
 */
void kbd_set_kstroke_handler(kbd_key_handler handler);


// TODO
//void kbd_set_repetition(int value);


#endif //_DEVICE_KEYBOARD_FX9860_KEYBOARD_H
