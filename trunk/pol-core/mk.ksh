#!/usr/bin/ksh

until CC=gcc-3.0 make -f makefile3.gcc
do
    echo Trying Again...
    sleep 10
done

