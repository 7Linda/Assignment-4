# Assignment 4

## Group Members

- Linda George
- Christopher Nguyen
- Arturo Sanchez
- Aidan Vuong

## Language

C++

## Description

This program uses System V message queues to send requests between a client and a server on Linux.

The server loads records from `namesDB.txt` into a hash table. It then creates worker threads and waits for client requests. The client sends record IDs to the server. If the server finds the ID, it sends the matching record back to the client.

The program uses pthreads, mutexes, and condition variables.

## Files

- `server.cpp`
- `client.cpp`
- `msg.cpp`
- `msg.h`
- `namesDB.txt`
- `Makefile`

## Requirements

This project should be run on Linux.

Required tools:

- `g++`
- `make`

## Compile

In the project folder, run:

```bash
make
```

This creates the server and client executables.

To clean and rebuild:

```bash
make clean
make
```

## Run

Start the server first:

```bash
./server namesDB.txt 3
```

The first argument is the database file. The second argument is the number of lookup threads.

Then open a second terminal in the same folder and run the client:

```bash
./client
```

The server must be running before the client.

## Stop the Server

In the server terminal, press:

```bash
Ctrl + C
```

This stops the server and removes the message queue.

## Database Format

The database file should be formatted like this:

```txt
id firstName lastName
```

## Notes

Running `make` only compiles the project. It does not automatically start the server.

The server will keep running while it waits for client requests.
