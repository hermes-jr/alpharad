#!/bin/sh
echo "Unique X coordinates: $(< ../points.log cut -d':' -f1 | sort -u | wc -l). If less than captured frame width, something is missing."
echo "Unique Y coordinates captured: $(< ../points.log cut -d':' -f2 | sort -u | wc -l). Must be equal to captured frame height."
printf "\n\n==============\n\n"
ent -c ../out.dat
printf "\n\n==============\n\n"
ent -b -c ../out.dat
# dieharder -g 201 -a -k 2 -Y 1 -f ../out.dat
