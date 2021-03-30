#!/bin/bash

# need root
[ "`id -u`" != "0" ] && { echo "need run as root"; exit; }

# tty color
[ -t 1 ] && y=$'\e[33m' && c=$'\e[0m'

# scan input
for f in /dev/input/event*; do 
	# evtest $f &
	evtest $f | awk -vf=$f -vy=$y -vc=$c ' $1=="Event:" && $0 ~/KEY_ENTER/ {print y f c": "$0; cmd="INPUT="f" ./key-vbox.sh "; print("== RUN daemon: "cmd); system(cmd)}' &
	echo $f;
done

# show msg
sleep 1; echo
# for i in `seq 3`; do
	# echo '== press Enter or Ctrl+C to exit (if stucked)..'
# done
echo
# read

echo '== EXIT scan'
pkill -ef '^evtest /dev/input/event'
echo
echo '== background running'
./key-vbox.sh stat
