#!/bin/sh

for num in `seq 1 9`; do
file="0${num}"
option="${file}_*.xml ./msc-${num}.xml"
eval "cp ${option}"
done
