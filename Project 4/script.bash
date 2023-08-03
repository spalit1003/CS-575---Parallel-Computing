#!/bin/bash
for ArrSize in 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304
do
	g++ all04.cpp -DARRAYSIZE=$ArrSize  -o proj4 -lm -fopenmp
	./proj4
done