# CPU Scheduler Simulation

A C++ operating system scheduling simulator that models how processes move through CPU, input, and output queues. The simulator supports First Come First Serve (FCFS) scheduling and optional Round Robin scheduling using a configurable time quantum.

## Features

- Simulates CPU process scheduling
- Supports FCFS scheduling
- Supports Round Robin scheduling with a custom quantum
- Uses Entry, Ready, Input, and Output queues
- Tracks CPU, input, output, and waiting time for each process
- Prints system state at regular time intervals
- Displays final simulation statistics

## Technologies Used

- C++
- STL `vector`
- STL `deque`
- File input
- Command-line arguments

## Operating System Concepts Demonstrated

- CPU scheduling
- Process queues
- Process state transitions
- CPU bursts
- I/O bursts
- Round Robin time slicing
- Waiting time
- CPU utilization
- System simulation

## How It Works

The program reads process data from an input file. Each process has an arrival time and a list of CPU, input, and output bursts.

During the simulation, processes move through several queues:

- Entry Queue: processes waiting to enter the system
- Ready Queue: processes waiting for CPU time
- Input Queue: processes waiting for the input device
- Output Queue: processes waiting for the output device

Only one process can use the CPU at a time. The input and output devices can also each handle one process at a time.

## Build

```bash
g++ -Wall -Wextra -std=c++11 cpu-scheduler-simulation.cpp -o cpu-scheduler-simulation