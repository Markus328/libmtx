# Matrices

This repository is about a very basic header-only lib for matrix operations. 
It implements some matrix and linear algebra algorithms such as matrix sum/subtraction/multiplication, matrix distance, LU decomposition, determinant and back/forward substitution for solving linear systems.

This project was inspired by GNU GSL.

# Tests

There are some definitions for tests in `tests` director. 
Not all systems have implementation of pthreads and getrandom() syscall, this test "suit" was only tested on Linux.



