# Producer-Consumer Problem — Multi-Threaded Simulation

![Language](https://img.shields.io/badge/Language-C-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Ubuntu%20Linux-orange.svg)
![Threads](https://img.shields.io/badge/Library-POSIX%20pthreads-green.svg)

A console-based simulation of the classical **Producer–Consumer problem** (bounded-buffer problem) using POSIX threads, a mutex, and two counting semaphores on Ubuntu Linux. Multiple producer threads generate data items and place them into a shared fixed-size buffer while multiple consumer threads remove and process those items — all safely synchronised to prevent race conditions, overflow, and underflow.

---

## 📑 Table of Contents

- [About the Project](#-about-the-project)
- [Features](#-features)
- [How It Works](#-how-it-works)
- [Requirements](#-requirements)
- [Project Structure](#-project-structure)
- [Installation & Build](#-installation--build)
- [Usage](#-usage)
- [Sample Output](#-sample-output)
- [Configuration](#-configuration)
- [Troubleshooting](#-troubleshooting)
- [Key Concepts](#-key-concepts)
- [License](#-license)

---

## 📖 About the Project

The Producer–Consumer problem is a classical synchronisation problem first described by Edsger Dijkstra in 1965. It models a scenario where:

- **Producers** generate data and place it into a shared buffer.
- **Consumers** take data from that buffer and process it.
- Both run concurrently, and the buffer has a fixed capacity.

Because multiple threads access the same memory simultaneously, synchronisation is required to avoid:

- ❌ **Race conditions** — two threads corrupting shared state
- ❌ **Buffer overflow** — producers writing when buffer is full
- ❌ **Buffer underflow** — consumers reading when buffer is empty
- ❌ **Deadlocks** — threads waiting on each other forever

This project implements the textbook solution using one **mutex** (for mutual exclusion) and two **counting semaphores** (to track empty and filled slots).

---

## ✨ Features

### Mandatory Requirements ✅
- Fixed-size shared buffer (circular queue)
- Multiple producer threads
- Multiple consumer threads
- Mutex-based mutual exclusion
- Semaphore-based synchronisation to prevent over/underflow

### Additional Features ✅
- 🎲 **Variable production/consumption rates** — random delays simulate realistic workloads
- 📝 **Timestamped logging** — every event tagged with `[HH:MM:SS]`
- 🎛️ **Dynamic buffer size** — user chooses buffer size and thread counts at runtime
- 🛑 **Graceful shutdown** — all threads joined, all resources released cleanly
- 📊 **Thread statistics** — per-thread and total item counts printed at end
- 👁️ **Live buffer visualisation** — ASCII snapshot of the buffer after every event

---

## ⚙️ How It Works

### The Algorithm

**Producer loop:**
```
produce an item
sem_wait(empty_sem)       // wait for an empty slot
pthread_mutex_lock(mutex) // enter critical section
  place item in buffer
  update in, count
pthread_mutex_unlock(mutex)
sem_post(full_sem)        // signal: one more filled slot
```

**Consumer loop:**
```
sem_wait(full_sem)        // wait for a filled slot
pthread_mutex_lock(mutex) // enter critical section
  remove item from buffer
  update out, count
pthread_mutex_unlock(mutex)
sem_post(empty_sem)       // signal: one more empty slot
consume the item
```

### Why This Order?

Calling `sem_wait` **before** `pthread_mutex_lock` is critical. If we reversed them, a producer could lock the mutex when the buffer is full, then block on `sem_wait(empty_sem)` while still holding the mutex — preventing any consumer from freeing a slot. That would be a **deadlock**.

### Synchronisation Primitives

| Primitive    | Initial Value | Role                                                       |
|--------------|---------------|------------------------------------------------------------|
| `mutex`      | unlocked      | Ensures only one thread accesses the buffer at a time     |
| `empty_sem`  | `BUFFER_SIZE` | Counts empty slots; producers wait on this when full      |
| `full_sem`   | `0`           | Counts filled slots; consumers wait on this when empty    |

---

## 🔧 Requirements

- **Operating System:** Ubuntu Linux (tested on 20.04 / 22.04 / 24.04) — or any Linux with POSIX support
- **Compiler:** GCC 9.0 or newer
- **Build tool:** GNU Make (optional)
- **Libraries:** POSIX threads (`libpthread`) and POSIX semaphores — both included with standard Linux

Install all prerequisites on a fresh Ubuntu system:

```bash
sudo apt update
sudo apt install build-essential
```

---

## 📁 Project Structure

```
producer-consumer/
├── pc.c          # Main C source code
├── Makefile      # Build automation
└── README.md     # This file
```

---

## 🚀 Installation & Build

### Clone the repository

```bash
git clone https://github.com/<your-username>/producer-consumer.git
cd producer-consumer
```

### Build with Make

```bash
make
```

### Or build manually with GCC

```bash
gcc -Wall -o pc pc.c -lpthread
```

> 💡 The `-lpthread` flag is **essential** — it links the POSIX threads library. Without it you'll get `undefined reference to pthread_create` linker errors.

---

## ▶️ Usage

Run the executable:

```bash
./pc
```

The program will prompt you to enter:

| Prompt                    | Example | Description                          |
|---------------------------|---------|--------------------------------------|
| Buffer size               | `5`     | Capacity of the shared buffer        |
| Number of producers       | `3`     | How many producer threads to create  |
| Number of consumers       | `2`     | How many consumer threads to create  |
| Items per producer        | `6`     | How many items each producer makes   |

Clean up build artifacts:

```bash
make clean
```

---

## 📺 Sample Output

```
====================================================
     PRODUCER - CONSUMER  SIMULATION  (pthreads)
====================================================

Enter buffer size            : 5
Enter number of producers    : 3
Enter number of consumers    : 2
Enter items per producer     : 6

 Buffer size      : 5
 Producers        : 3 (each produces 6 items)
 Consumers        : 2
 Total items      : 18

### SIMULATION STARTS ###

[06:25:47] Producer 2 PRODUCED item 2000  -> Buffer [ 2000   .     .     .     .   ]  1/5
[06:25:47] Consumer 0 CONSUMED item 2000  <- Buffer [  .     .     .     .     .   ]  0/5
[06:25:48] Producer 0 PRODUCED item 0     -> Buffer [    0   .     .     .     .   ]  1/5
[06:25:48] Producer 1 PRODUCED item 1000  -> Buffer [    0  1000   .     .     .   ]  2/5
...
[06:25:49] Producer 2 PRODUCED item 2003  -> Buffer [ 1001  2002     2  1002  2003 ]  5/5  ← FULL
[06:25:49] Consumer 0 CONSUMED item 1001  <- Buffer [ 2002     2  1002  2003   .   ]  4/5
...
[06:25:51] >>> Producer 0 SHUTTING DOWN (produced 6 items total)
[06:25:54] >>> Consumer 1 SHUTTING DOWN (consumed 9 items total)

====================================================
                 SIMULATION SUMMARY
====================================================
 Total items produced : 18
 Total items consumed : 18
 Items left in buffer : 0
 Producer 0 produced  : 6 items
 Producer 1 produced  : 6 items
 Producer 2 produced  : 6 items
 Consumer 0 consumed  : 9 items
 Consumer 1 consumed  : 9 items
====================================================
```

**Things to notice:**
- Every line is timestamped and shows a live snapshot of the buffer.
- When the buffer reaches `5/5`, it is full — the next event must be a `CONSUMED`.
- When the buffer reaches `0/5`, it is empty — the next event must be a `PRODUCED`.
- At the end, **total produced = total consumed** — proof that synchronisation works and no item was lost or duplicated.

---

## 🎛️ Configuration

All parameters are entered at runtime, so you can easily experiment with different scenarios without recompiling:

| Scenario                            | Buffer | Producers | Consumers | Items/Producer | What You'll Observe                |
|-------------------------------------|--------|-----------|-----------|----------------|------------------------------------|
| Balanced                            | 5      | 3         | 2         | 6              | Smooth flow, occasional waits      |
| Tight buffer, one slow consumer     | 2      | 4         | 1         | 5              | Producers block frequently         |
| Many consumers, few producers       | 10     | 1         | 5         | 20             | Consumers often wait for items     |
| Stress test                         | 20     | 10        | 10        | 50             | Heavy concurrency, all synchronised|

---

## 🛠️ Troubleshooting

**Problem:** `undefined reference to pthread_create` when compiling.
**Solution:** You forgot the `-lpthread` flag. Use `gcc -Wall -o pc pc.c -lpthread`.

**Problem:** `permission denied` when running `./pc`.
**Solution:** Make it executable with `chmod +x pc`.

**Problem:** Program hangs and never finishes.
**Solution:** Check your input values — all must be positive integers. The program divides total items evenly among consumers, so no thread should wait forever.

**Problem:** `make: command not found`.
**Solution:** Install build tools with `sudo apt install build-essential`.

---

## 📚 Key Concepts

If you're new to multi-threaded programming, these are the core ideas behind this project:

- **Thread** — a unit of execution within a process that shares memory with other threads.
- **Race condition** — a bug where the output depends on the unpredictable order of concurrent thread execution.
- **Critical section** — a block of code that accesses shared data; only one thread may execute it at a time.
- **Mutex (mutual exclusion lock)** — a binary lock that serialises access to a critical section.
- **Semaphore** — a non-negative integer counter used for signalling between threads.
- **Deadlock** — a state where two or more threads wait on each other forever.
- **Circular buffer** — an array used as a queue where indices wrap around using `(i + 1) % N`.

---

## 📄 License

This project is released under the MIT License. Feel free to use, modify, and learn from it.

---

## 👤 Author

Created as part of an Operating Systems course project demonstrating concurrency and synchronisation in C.

---
