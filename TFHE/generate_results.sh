#!/bin/bash
gcc user_testing.c -o user_testing -ltfhe-spqlios-fma
gcc cloud_testing.c -o cloud_testing -ltfhe-spqlios-fma
gcc verify_testing.c -o verify_testing -ltfhe-spqlios-fma
./user_testing
sage -python equation_solver.py
python verify.py
