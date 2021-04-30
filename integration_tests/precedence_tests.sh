#!/bin/sh
g++ ../src/main.cpp
./a.out (echo A && echo B) || (echo C && echo D)