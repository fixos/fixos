#!/bin/sh
#
# Tool script used to strip all unshared symbol from a shared library.
# A symbol is unshared when it is not GLOBAL and/or its visibility is not
# DEFAULT (so only exported symbol should be marked with this visibility).

if [ $# -ne 2 ]
then
	echo "Usage: $0 <source-object> <dest-object>"
	exit 1
fi


shared="`extract_shsym.sh func $1` `extract_shsym.sh object $1`"

for sym in $shared
do
	nostrip="$nostrip -K $sym"
done

nostrip="$nostrip -K RELOC_GOT_E -K RELOC_GOT_B"

echo Not stripped symbols : "$nostrip"

$OBJCOPY -S $nostrip $1 $2
