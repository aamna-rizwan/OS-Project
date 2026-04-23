=============================================================
              PRODUCER-CONSUMER SIMULATION
=============================================================

FILES IN THIS FOLDER
--------------------
  pc.c         - Main C source code (everything is here)
  Makefile     - Build automation
  README.txt   - This file

---------------------------------------------------------------
ONE-TIME SETUP ON UBUNTU
---------------------------------------------------------------
Open a terminal (Ctrl + Alt + T) and run:

    sudo apt update
    sudo apt install build-essential

This installs gcc and make. The pthread and semaphore libraries
come with Ubuntu by default, no separate install needed.

Check everything is ready:
    gcc --version
    make --version

---------------------------------------------------------------
HOW TO BUILD
---------------------------------------------------------------
Place pc.c and Makefile in the same folder, then:

    cd /path/to/folder
    make

Or without the Makefile:
    gcc -Wall -o pc pc.c -lpthread

The flag -lpthread is MANDATORY. It links the POSIX threads
library that provides pthread_create, mutex, semaphores, etc.

---------------------------------------------------------------
HOW TO RUN
---------------------------------------------------------------
    ./pc

You will be prompted to enter:
  - buffer size         (e.g. 5)
  - number of producers (e.g. 3)
  - number of consumers (e.g. 2)
  - items per producer  (e.g. 6)

The simulation then runs, printing a live log of every
produce/consume event plus a snapshot of the buffer, followed
by a summary of statistics when it finishes.

---------------------------------------------------------------
HOW TO CLEAN
---------------------------------------------------------------
    make clean
