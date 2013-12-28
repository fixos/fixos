#ifndef _DEVICE_TERMINAL_GENERIC_EARLY_TERM_H
#define _DEVICE_TERMINAL_GENERIC_EARLY_TERM_H

/**
 * Interface and definitions for the early terminal, used to print informations
 * during first boot steps.
 * The interface need to be implemented by platform-specific files.
 *
 * The goal of these functions is to allow to use display before initialization
 * of high-level part of the kernel, such as device registering and VFS.
 *
 * Position is incremented automaticaly, and "scroll" must be done internaly
 * if a new line begin at end of screen.
 */


/**
 * Colors used by the terminal, implementation must choose the closest available
 * color if the given one isn't.
 */
#define EARLYTERM_COLOR_WHITE	0
#define EARLYTERM_COLOR_BLACK	1


/**
 * Initialize the early terminal, using the given VRAM if needed by implementation.
 * TODO better design : VRAM size is not abstracted...
 */
void earlyterm_init(void *vram);


/**
 * Print the given string into the terminal.
 */
void earlyterm_write(const char *str);

/**
 * Change the terminal character colors.
 */
void earlyterm_set_colors(int front_c, int back_c);

/**
 * Clear the terminal (and put the cursor in (1,1) )
 * WARNING : the terminal is filled with the back color used
 */
void earlyterm_clear();

/**
 * Move the cursor at the given position, (1,1) is top-left corner.
 */
void earlyterm_set_pos(int posx, int posy);

/**
 * Get current position.
 */
int earlyterm_posx();
int earlyterm_posy();

#endif //_DEVICE_TERMINAL_GENERIC_EARLY_TERM_H
