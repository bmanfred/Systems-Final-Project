#!/bin/sh

# Shell script for experiment 1, testing 
HAMMERS=1
THROWS=1
VERBOSE=0




while [ $# -gt 0 ]; do
	case $1 in
		-h) HAMMERS=$2;shift;;
		-t) THROWS=$2;shift;;
		-v) VERBOSE=1;;
	esac
	shift
done
echo $HAMMERS
echo $THROWS
echo $VERBOSE
