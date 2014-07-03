#!/bin/sh
#
# Tool script used to extract symbol names marked as exported symbol for shared
# library (GLOBAL binding and DEFAULT visibility).

if [ $# -ne 2 ]
then
	echo "Usage: $0 (object|func) <object-file>"
	exit 1
fi

case $1 in
object|func|both) extract=$1 ;;
*)	echo "Bad arguments : only accepted ar (object|func)"
	exit 1
esac


$READELF -s -W $2 | {
	#ignore first 3 lines
	read ; read ; read ;

	read line
	ret=$?
	while [ $ret -eq 0 ]
	do
		# field are  "Num:    Value  Size Type    Bind   Vis      Ndx Name"
		symbind=`echo $line | cut -f 5 -d ' '`
		symvis=`echo $line | cut -f 6 -d ' '`

		
		if [ $symbind == GLOBAL -a $symvis == DEFAULT ]
		then 
			symtype=`echo $line | cut -f 4 -d ' '`
			symname=`echo $line | cut -f 8 -d ' '`

			if [ $symtype == OBJECT -a $extract == object ]
			then
				echo $symname
			elif [ $symtype == FUNC -a $extract == func ]
			then
				echo $symname
			fi
		fi

		read line
		ret=$?
	done

}
