#!/bin/bash

umount /dev/CFPGA251
if ! rmmod $1 ; then
	echo "Unable to unload the module: ${1}" >&2
	exit 1
fi

echo "Module $1 successfully unloaded"
