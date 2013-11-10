/**
 * This is the first test of a user process (for now it's a  piece of code
 * compiled and linked within the kernel itself, not really an independant
 * binary).
 */

int usertest_main() __attribute__ (( section(".user.pretext") ));

int usertest_main() {
//	asm volatile ("mov.l ");

	asm volatile ("trapa #42");

	// never return
	while(1);

	return 0;
}
