PATH="$PATH:/usr/share/gnush_v11.02_elf-1/bin"
C_FILES="arch/sh/interrupt.c arch/sh/exception_handler.S arch/sh/interrupt_handler.S arch/sh/tlb_handler.S keyboard/iskeydown.c gcc_fix/udivsi3_i4i.S init.c bootstrap.s initialize.s display/T6K11/drawall.s display/T6K11/setpixel.s display/T6K11/terminal.c sys/terminal.c"
G1A_NAME="fixos.g1a"
GNU85_ROOT="/home/leo/GNU85"

INCLUDE_DIRS="-I$GNU85_ROOT/include/sh -I$GNU85_ROOT/include/revolution -I$GNU85_ROOT/include/fxlib"
LIB_DIRS="-L$GNU85_ROOT/lib"
LIBRARIES=""

sh-elf-gcc -m3 -mb -Os -nostdlib -T"fixos.ld" $C_FILES $LIB_DIRS $INCLUDE_DIRS $LIBRARIES -o myaddin.elf
sh-elf-objcopy -R .comment -R .bss -O binary myaddin.elf myaddin.bin
~/GNU85/g1awrapper/g1awrapper myaddin.bin -o $G1A_NAME -i icon.bmp
sleep 1000
