# Dynamic Memory Allocation

## Overview
This project implements an emulation of memory management system, more precisely, malloc and free functions of C language. 
Memory management algorithm is buddy system with optimization by AVL binary tree structure (std::map and std::set were restricted in the testing environment of my university).

## Interface

- **HeapInit**: initializes memory pool, inserts the required metadata into available memory slots. Parameters are memory pool starting address and memory pool size.

- **HeapAlloc**: allocates memory block of required size, returns address of the allocated block.

- **HeapFree**: frees the provided memory address, returns boolean value that indicates whether the memory was freed or not.

- **HeapDone**: frees the allocated block, opposite of HeapInit.
