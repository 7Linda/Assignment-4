# Assignment 4

## Group Members

- Linda George
- Christopher Nguyen
- Arturo Sanchez
- Aidan Vuong

## Programming Language

C++

## Project Description

This project demonstrates client-server communication using System V message queues in Linux. The server loads records from a database file into a hash table, creates multiple worker threads, and waits for client requests. The client sends random record IDs to the server. The server searches for the requested ID and sends the matching record back to the client if it exists.

The program also uses pthreads, mutexes, and condition variables to safely manage shared data between multiple threads.

## Files

- `server.cpp` - Runs the server, loads the database, manages the hash table, creates worker threads, and processes client requests.
- `client.cpp` - Runs the client and sends lookup requests to the server.
- `msg.cpp` - Contains the message queue helper functions.
- `msg.h` - Contains message queue definitions and function declarations.
- `namesDB.txt` - Database file containing IDs, first names, and last names.
- `Makefile` - Used to compile the project.

## Requirements

This program is intended to run on Linux because it uses System V message queues.

You need:

- Linux
- g++
- make
- pthread support

## How to Compile

From the project folder, run:

```bash
make