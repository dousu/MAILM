#!/bin/sh

for num in `seq 101 300`; do
wget "http://gttm.jp/gttm/wp-content/uploads/2015/12/msc-${num}.xml"
done
