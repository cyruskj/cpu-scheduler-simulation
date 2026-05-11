# CPU Scheduler Simulation

A C++ operating system scheduling simulator that models how processes move through CPU, input, and output queues. The simulator supports both First Come First Serve (FCFS) and Round Robin (RR) scheduling algorithms using a configurable time quantum.

This project simulates how an operating system scheduler manages processes, CPU bursts, input/output bursts, waiting time, and resource utilization over time.

---

## Features

- Simulates CPU process scheduling
- Supports FCFS scheduling
- Supports Round Robin scheduling with configurable quantum
- Uses Entry, Ready, Input, and Output queues
- Simulates dedicated input and output devices
- Tracks CPU, input, output, and waiting time for each process
- Displays queue states during execution
- Calculates final scheduling statistics
- Supports multiple workload input files

---

## Technologies Used

- C++
- STL `vector`
- STL `deque`
- File input handling
- Command-line arguments
- Console-based simulation

---

## Operating System Concepts Demonstrated

- CPU scheduling
- FCFS scheduling
- Round Robin scheduling
- Process state transitions
- CPU bursts
- Input/Output bursts
- Queue management
- Waiting time
- CPU utilization
- System simulation
- Time slicing

---

## Project Structure

```txt
cpu-scheduler-simulation/
├── README.md
├── Makefile
├── cpu-scheduler-simulation.cpp
├── screenshots/
│   ├── mixed-workload-output.png
│   ├── stress-test-output.png
│   └── statistics-summary.png
├── input/
│   ├── input.txt
│   ├── heavy-cpu.txt
│   ├── heavy-io.txt
│   ├── short-jobs.txt
│   ├── mixed-workload.txt
│   └── stress-test.txt
````

---

## How It Works

The simulator reads process information from an input file. Each process contains:

* Process name
* Arrival time
* CPU bursts
* Input bursts
* Output bursts

Processes move through multiple queues during execution:

* **Entry Queue** → Waiting to enter the system
* **Ready Queue** → Waiting for CPU execution
* **Input Queue** → Waiting for the input device
* **Output Queue** → Waiting for the output device

The scheduler updates the simulation one clock tick at a time until all processes terminate or the maximum simulation time is reached.

---

## Build

```bash
g++ -Wall -Wextra -std=c++11 cpu-scheduler-simulation.cpp -o cpu-scheduler-simulation
```

---

## Run

Run using the default Round Robin quantum:

```bash
./cpu-scheduler-simulation input/input.txt
```

Run with a custom quantum:

```bash
./cpu-scheduler-simulation input/input.txt 10
```

---

## Example Workloads

### CPU-Heavy Workload

```bash
./cpu-scheduler-simulation input/heavy-cpu.txt
```

### I/O-Heavy Workload

```bash
./cpu-scheduler-simulation input/heavy-io.txt
```

### Mixed Workload

```bash
./cpu-scheduler-simulation input/mixed-workload.txt
```

### Stress Test

```bash
./cpu-scheduler-simulation input/stress-test.txt 5
```

---

## Example Output

```txt
--- Time 125 ---
Active: 110
IActive: 116
OActive: 114

Entry Q: 123 125 127
Ready Q: 118
Input Q: 109
Output Q: [Empty]
```

---

## Final Statistics Example

```txt
========== Final Simulation Summary ==========
Final Timer: 211
Processes Terminated: 12
CPU Idle Time: 4
CPU Utilization: 98.10%
Average Waiting Time: 43.00
Total CPU Time: 207
Total Input Time: 170
Total Output Time: 117
Processes Left in Entry Queue: 0
Processes Left in Ready Queue: 0
Processes Left in Input Queue: 0
Processes Left in Output Queue: 0
=============================================
```

---

## Scheduling Algorithm Comparison

The simulator can be used to compare FCFS and Round Robin scheduling behavior by changing the quantum value.

### FCFS-like Execution

```bash
./cpu-scheduler-simulation input/mixed-workload.txt 999
```

### Round Robin Execution

```bash
./cpu-scheduler-simulation input/mixed-workload.txt 5
```

Different scheduling strategies affect:

* Waiting time
* CPU utilization
* Process responsiveness
* Queue buildup
* Overall throughput

---

## What I Learned

This project improved my understanding of:

* Operating system scheduling algorithms
* Queue-based process management
* CPU and I/O burst handling
* Simulation-driven programming
* Resource utilization tracking
* STL containers in C++

I also gained more experience designing larger multi-function C++ programs and organizing simulation logic into modular components.

---

## Future Improvements

* Add graphical scheduling timeline visualization
* Export statistics to CSV files
* Add additional scheduling algorithms
* Add process priority support
* Improve process state tracking
* Add multi-core CPU simulation