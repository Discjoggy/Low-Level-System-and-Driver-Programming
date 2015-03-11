#!/bin/bash

if ! lsmod | grep "${1}" ; then
	if ! insmod $1.ko ; then
		echo "Unable to load the module: ${1}" >&2
		exit 1
	else
		echo "Module $1 successfully unloaded"
	fi
else
	echo "Module ${1} already loaded"
fi

if [ $2 = "format" ] ; then
	mkfs.vfat -F 32 /dev/CFPGA251 -n RAMDISK
     	#mkdir /media/student/RAMDISK1
	#mount /dev/CFPGA251 /media/student/RAMDISK1
	echo "disk successfully formatted as FAT" >&2
fi
