#!/bin/bash

for ((i = 1; i < 2000; i = i*2)); do
    for ((j = 1; j < 100000; j = j*2)); do
        ./tlb $i $j
    done
done

for((i=1200; i < 2048; i = i+200)); do
    for ((j = 1; j < 100000; j = j*2)); do
        ./tlb $i $j
    done
done
