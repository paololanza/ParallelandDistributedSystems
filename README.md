# Parallel and Distributed Systems

This repository contains the project for the exam Parallel and Distributed Systems.

The project requires to implement three different implementations of Huffman encoding:
* Sequential, it is a simple implementation without considering any parallelization of the code.
* Native Thread, this implementation parallelize all the possible tasks of the encoding by using native threads and a threadpool to manage resources.
* Fast Flow, this implementation parallelize all the tasks by using FastFlow library to manage the parallelization. 
