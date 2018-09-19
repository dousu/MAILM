#!/bin/sh

for num in `seq 101 267`; do
unzip -o -qq "${num}.zip"
folder="${num}"
option="${folder}/MSC*.xml ./"
eval "cp ${option}"
done
