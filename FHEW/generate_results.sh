#!/bin/bash
cp FHEW.cpp results_FHEW.cpp FHEW
cd FHEW
g++ -w -ansi -Wall -O3 -o results_FHEW results_FHEW.cpp -L. -lfhew -lfftw3 -std=c++11
./results_FHEW
mv Number_of_decryptions.csv ..
mv gaussian_elimination.txt ..
mv secret_key.txt ..
cd ..
sage -python equation_solver.py
python verify.py
