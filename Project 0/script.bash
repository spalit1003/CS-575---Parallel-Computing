#!/bin/bash
#number of threads: 
for t in 1 4
do
echo NUMT = $t
g++ -DNUMT=$t proj0.cpp -o proj0 -lm -fopenmp 
./proj0
done

#g++-12 -DNUMT=$t proj0.cpp -o proj0 -lm -fopenmp 