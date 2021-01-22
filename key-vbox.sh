#!/bin/bash

# -----
echoR(){ echo -e "\e[31m$@\e[0m"; }
echoG(){ echo -e "\e[32m$@\e[0m"; }
echoY(){ echo -e "\e[33m$@\e[0m"; }
echoH(){ echo -e "\e[01;32;40m$@\e[0m"; }


# -----
dir=`dirname $(readlink -f $BASH_SOURCE)`
#echo dir: $dir

start(){
	echoG "$FUNCNAME $@"

	stat && return
	sudo id && {
		sudo nohup $dir/keyevent $INPUT &>/dev/null &
		echo "running at bg..."
	}
}
stat(){
	echoG $FUNCNAME
	pgrep -a 'keyevent'
}
stop(){
	echoG $FUNCNAME
	sudo pkill -e 'keyevent'
}
restart(){
	echoG "$FUNCNAME $@"
	stop
	start "$@"
}

act=${1:-start}
shift

[ "$act" == "-h" ] && {
cat <<EOF
	stat        show status
	stop        stop service
	[start]     start service
	restart     restart service
EOF
	exit
}

$act "$@"

