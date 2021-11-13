#! /bin/bash

x_window() {
	local x0=$1
	local y0=$2
	local w=$3
	local h=$4
	local color=$5
	local color2=$6

	printf "3:%d|%d|%d|%d|%d" ${x0} ${y0} ${w} ${h} ${color} > /dev/display
	printf "3:%d|%d|%d|%d|%d" ${x0} ${y0} ${w} $(( h/10 ))  ${color2} > /dev/display
	printf "3:%d|%d|%d|%d|%d" $((x0 + h/20)) $((y0+h/10 + h/20 )) $((w - (h/20)*2 ))  $(( h - h/10 - (h/20)*2 ))  65535 > /dev/display
	printf "3:%d|%d|%d|%d|%d" $(( x0 + w - h/10 + 2)) $(( y0 + 2 )) $(( h/10 - 2*2 )) $(( h/10 - 2*2 ))  63488  > /dev/display
	printf "3:%d|%d|%d|%d|%d" $(( x0 + w - (h/10)*2 + 2)) $(( y0 + 2 )) $(( h/10 - 2*2 )) $(( h/10 - 2*2 ))  2016  > /dev/display
	
	printf "2:%d|%d|%d|%d" ${x0} ${y0} $((w/2-1)) $((h/2-1)) > /dev/display
	printf "1:1" > /dev/display
	sleep 0.1


	printf "2:%d|%d|%d|%d" $((x0+w/2+2)) $((y0)) $((w/2-1)) $((h/2-1)) > /dev/display
	printf "1:1" > /dev/display
	sleep 0.1

	printf "2:%d|%d|%d|%d" $((x0)) $((y0+h/2)) $((w/2-1)) $((h/2-1)) > /dev/display
	printf "1:1" > /dev/display
	sleep 0.1

	printf "2:%d|%d|%d|%d" $((x0+w/2+2)) $((y0+h/2+2)) $((w/2-1)) $((h/2-1)) > /dev/display
	printf "1:1" > /dev/display
	sleep 0.1

	printf "1:0" > /dev/display
}

x_window 50 50 100 75 0 31
x_window 100 100 100 75 0 63519
x_window 80 10 100 75 0 2047
