# System V IPC — Shared Memory & Message Queue

A simple C++ project created for **training and understanding Linux IPC (Inter-Process Communication)** infrastructure.

The project demonstrates how two independent processes can communicate using:

* **System V Shared Memory** — used to share the actual data.
* **System V Message Queue** — used for synchronization between processes.

## Purpose

The main goal of this project is to understand the basic infrastructure and workflow of System V IPC:

```text
Producer
   |
   | Write data
   v
Shared Memory
   ^
   |
   | Read data
   |
Consumer

Message Queue
   |
   +---- DATA_READY ----> Consumer
   |
   +---- DATA_READ  <---- Producer
```

The message queue is used for synchronization, while the shared memory segment is used to transfer the actual data.

## Example

The producer writes a message into shared memory and sends a `DATA_READY` notification through the message queue.

The consumer waits for the notification, reads the data from shared memory, and sends a `DATA_READ` acknowledgment back to the producer.

```text
Producer                         Consumer
   |                                |
   |-- Write to shared memory ----->|
   |                                |
   |-- DATA_READY ----------------->|
   |                                |
   |                          Read shared memory
   |                                |
   |<--------- DATA_READ -----------|
   |                                |
```

## Build

The project uses CMake.

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

This creates:

```text
producer
consumer
```

## Run

Create the file used by `ftok()`:

```bash
touch /tmp/my_ipc_file
```

Start the producer:

```bash
./producer "Hello from producer"
```

The producer will wait for the consumer.

In another terminal, start:

```bash
./consumer
```

The consumer reads the message from shared memory and acknowledges it through the message queue.

## Technologies

* C++
* Linux
* System V IPC
* Shared Memory
* Message Queue
* CMake

This is a **training/example project** intended to provide a basic understanding of Linux IPC mechanisms and their interaction.
