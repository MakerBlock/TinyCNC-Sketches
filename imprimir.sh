#!/bin/bash
#
#	Imprimir desde la terminal un gcode
#	AGPLv3 © Ernesto Bazzano
#
PUERTO=$(ls /dev/ttyUSB* | head -n1)
echo $PUERTO
stty -F $PUERTO cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
T=$(cat $1 | grep ^G | wc -l)
cat $1 | while read A; do
	N=$((N+1))
	echo $((100/$T*$N)) > /dev/stderr
	echo $A > /dev/stderr
	echo $A
	sleep .3
done > $PUERTO
