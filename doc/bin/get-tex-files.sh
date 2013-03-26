#!/bin/sh

FLIST="$1"

while :;
do
    LAST="$FLIST"
    for F in $LAST; do
	NL=`cat $F | grep \\input{ | grep -v ^% | cut -d{ -f2 | cut -d} -f1`
	for F2 in $NL; do
	    FLIST="$FLIST $F2.tex"
	done
    done
    FLIST=`ls $FLIST 2>/dev/null | sort | uniq`

    if [ "$LAST" = "$FLIST" ]; then
	echo $LAST
	exit 0
    fi
done
