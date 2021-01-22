#!/bin/bash

# need root
[ "`id -u`" != "0" ] && { echo "need run as root"; exit; }

# scan input
for f in /dev/input/event*; do 
	# evtest $f &
	evtest $f | awk -vf=$f '{print f" :"$0}' &
	echo $f;
done

# show msg
sleep 1; echo
for i in `seq 3`; do
	echo '== press Enter or Ctrl+C to exit..'
done
echo
read

echo '== exit'
pkill -ef '^evtest /dev/input/event'
