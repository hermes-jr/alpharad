#!/bin/bash
echo "Unique X coordinates: $(cat ../points.log | cut -d':' -f1 | sort -u | wc -l). If less than captured frame width, something is missing."
echo "Unique Y coordinates captured: $(cat ../points.log | cut -d':' -f2 | sort -u | wc -l). Must be equal to captured frame height."
printf "\n\n==============\n\n"
ent -c ../out.dat
printf "\n\n==============\n\n"
ent -b -c ../out.dat
