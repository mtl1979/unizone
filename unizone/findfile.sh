#!/bin/sh
NAME=`basename $2 | sed 's/\.\(c\|cpp\)/.o/'`
DIR=`dirname $2`

PATTERN="s|^$NAME:|$DIR/$NAME:|"
sed $PATTERN < $1 > $1.tmp
mv -f $1.tmp $1
