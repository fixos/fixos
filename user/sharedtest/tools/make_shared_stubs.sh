#!/bin/sh
#
# Tool script used to create dynamic library function stubs.
# Each stub is a short assembly file, containing the function name and some
# stuff to allow lazy symbol resolution at runtime.
# <soname> is the name of the shared library which will be marked as needed
# by any executable using at least one of the exported functions (usualy
# it should be the same as <object-file>).
# <dest-dir> is the destination directory for all stub file generated.

if [ $# -ne 3 ]
then
	echo "Usage: $0 <soname> <object-file> <dest-dir>"
	exit 1
fi


func="`extract_shsym.sh func $2`"

mkdir $3

for sym in $func
do
	echo "Creating stub for '$sym'"

	echo "
		.set $sym, "$sym"__plt
		.extern dynld_solvename_call
		.extern SONAME_$1

		.hidden $sym
		.type $sym, function
		.global $sym

		! to be sure SONAME_libname is undefined in object file
		.set useless, SONAME_$1

		.section \".plt\"
		.align 2

	"$sym"__plt:
		mov.l pltcache, r1
		mova pltcache, r0
		jmp @r1
		nop	! to be 4-bytes aligned
	pltcache:
		.long dynld_solvename_call
	"$sym"__dynstr:
		.asciz \"$sym\"
" > $3/"$sym"_stub.s
done


# Create the Shared Object name string
echo "
		.global SONAME_$1
		.section \".libraries\"

	SONAME_$1:
		.asciz \"$1\"

" > $3/soname.s


