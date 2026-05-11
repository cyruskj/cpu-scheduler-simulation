/***********************************************************************
* CPU Scheduling Simulator
* Author: Cyrus Johnson
*
* Simulates operating system CPU scheduling using:
*   - First Come First Serve (FCFS)
*   - Round Robin Scheduling
*
* Features:
*   - CPU, Input, and Output device simulation
*   - Multiple process queues
*   - Process statistics tracking
*   - Configurable Round Robin quantum
*
* Technologies:
*   - C++
*   - STL deque/vector
*
************************************************************************/

#include <iostream>
#include <vector>
#include <deque> 
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;

/* Constants */
const int MAX_TIME = 500;       // The maximum time for the simulation
const int IN_USE = 5;           // The number of processes that can be in use at once
const int HOW_OFTEN = 25;       // How often to print the system state
const int DEFAULT_QUANTUM = 5;   // The default time quantum for Round Robin

/* Process Structure */
struct Process {
    string process_name;                // The name of the process
    int process_id;                     // The ID of the process
    int arrival_time;                   // The time the process arrives

    vector<pair<char, int>> history;    // Vector to store the process's history

    int history_index = 0;              // Index into the history vector

    int cpu_timer = 0;                  // Timer for CPU usage
    int input_timer = 0;                // Timer for input usage
    int output_timer = 0;               // Timer for output usage

    int total_cpu_time = 0;             // Total time spent on CPU
    int total_input_time = 0;           // Total time spent on input
    int total_output_time = 0;          // Total time spent on output

    int cpu_count = 0;                  // Number of times the process has used the CPU
    int input_count = 0;                // Number of times the process has used input
    int output_count = 0;               // Number of times the process has used output

    int waiting_time = 0;               // Total time spent waiting
};

/* Queues */
deque<Process*> entry_queue;        // Queue for new processes
deque<Process*> ready_queue;        // Queue for ready processes
deque<Process*> input_queue;        // Queue for processes waiting for input
deque<Process*> output_queue;       // Queue for processes waiting for output

/* Active Processes */
Process* active = nullptr;          // Pointer to the currently running process
Process* input_active = nullptr;    // Pointer to the currently waiting for input process
Process* output_active = nullptr;   // Pointer to the currently waiting for output process

/* Simulation Statistics */
int sys_timer = 0;                  // The system timer
int quantum = DEFAULT_QUANTUM;      // The time quantum for Round Robin
int quantum_counter = 0;            // The current time within the quantum
int idle_ticks = 0;                 // The number of idle ticks
int total_done = 0;                 // The total number of processes that have completed

int total_waiting_time = 0;         // The total time all processes have spent waiting
int total_cpu_time = 0;             // The total time all processes have spent on the CPU
int total_input_time = 0;           // The total time all processes have spent on input
int total_output_time = 0;          // The total time all processes have spent on output

/* Function Prototypes */
void read_input_file(string filename);
void run_simulation();
void check_arrivals();
void dispatch_cpu();
void handle_cpu();
void handle_input();
void handle_output();
void update_waiting_time();
void print_state(int t);
void terminate_process(Process* p);

/*********************************************************************************
* read_input_file
*   Function to read the input file and populate the entry queue with processes
*
* @param filename - The name of the input file to read
*********************************************************************************/
void read_input_file(string filename) {
    // Open the file
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "Failed to open " << filename << " file!" << endl;
        exit(1);
    }

    // Read the file and create processes
    string name;            // The name of the process
    int arrival;            // The arrival time of the process
    int next_pid = 101;     // The next available process ID

    // Read until we hit STOPHERE
    while (infile >> name && name != "STOPHERE") {
        if (!(infile >> arrival)) break; 

        // Create a new process and fill in the details
        Process* p = new Process();
        p->process_name = name;
        p->arrival_time = arrival;
        p->process_id = next_pid++;
        p->history_index = 0;

        char type;  // The type of the operation
        int val;    // The value of the operation
        
        // Read the history until we hit N
        while (infile >> type && type != 'N') {
            infile >> val;
            p->history.push_back(make_pair(type, val));
        }
        
        // Read the final value
        infile >> val; 

        // Add to entry queue if it has a history, otherwise clean up memory
        if (!p->history.empty()) {
            entry_queue.push_back(p);
        } else {
            delete p; 
        }
    }
    // Close the file
    infile.close();
}

/*********************************************************************************
* main 
*   The main function. Loop represents the scheduler in the OS. Timer initialized
*   to '0' and increments until the timer reaches MAX_TIME or there are no more
*   processes left.
*
* @param argc - The number of command line arguments
* @param argv - The array of command line arguments
*********************************************************************************/
int main (int argc, char* argv[]){
    // Check the command line arguments
    if (argc < 2) {
        cout << "Usage: ./cpu-scheduler-simulation <file> [quantum]" << endl;
        return 1;
    }

    // Read the quantum if provided
    if (argc == 3) {
        quantum = stoi(argv[2]);
    }

    // Read the input file and populate the entry queue
    read_input_file(argv[1]);
    run_simulation();

    // Print final stats
    cout << "\n========== Final Simulation Summary ==========" << endl;
    cout << "Final Timer: " << sys_timer << endl;
    cout << "Processes Terminated: " << total_done << endl;
    cout << "CPU Idle Time: " << idle_ticks << endl;

    // Calculate and print CPU utilization
    if (sys_timer > 0) {
        double cpu_utilization = ((double)(sys_timer - idle_ticks) / sys_timer) * 100.0;
        cout << fixed << setprecision(2);
        cout << "CPU Utilization: " << cpu_utilization << "%" << endl;
    }

    // Calculate and print average waiting time
    if (total_done > 0) {
        double average_waiting_time = (double)total_waiting_time / total_done;
        cout << "Average Waiting Time: " << average_waiting_time << endl;
    }

    // Print the total CPU time, input time, and output time
    cout << "Total CPU Time: " << total_cpu_time << endl;
    cout << "Total Input Time: " << total_input_time << endl;
    cout << "Total Output Time: " << total_output_time << endl;
    cout << "Processes Left in Entry Queue: " << entry_queue.size() << endl;
    cout << "Processes Left in Ready Queue: " << ready_queue.size() << endl;
    cout << "Processes Left in Input Queue: " << input_queue.size() << endl;
    cout << "Processes Left in Output Queue: " << output_queue.size() << endl;
    cout << "===============================================" << endl;

    return 0; // Return 0 to indicate successful execution
}

/*********************************************************************************
* run_simulation
*   This is the main loop that runs the whole thing until max time or we
*   run out of processes to run.
*
* @param none
*********************************************************************************/
void run_simulation() {
    // Loop until max time or we run out of process to run
    while (sys_timer < MAX_TIME) {
        check_arrivals();
        handle_cpu();
        handle_input();
        handle_output();
        dispatch_cpu();
        update_waiting_time();

        // Print the state every HOW_OFTEN time units
        if (sys_timer % HOW_OFTEN == 0) {
            print_state(sys_timer);
        }

        // Increment the timer
        sys_timer++;

        // Stop if nothing left to do
        if (entry_queue.empty() && ready_queue.empty() && input_queue.empty() && 
            output_queue.empty() && !active && !input_active && !output_active) {
            break;
        }
    }
}

/*********************************************************************************
* check_arrivals
*   Checks if any new process arrived in the entry queue and moves them to the ready queue
*   if there is room in the system.
*
* @param none
*********************************************************************************/
void check_arrivals() {
    // Check the entry queue for new arrivals and move them to ready if there is room in the system
    int cur_in_sys = (int)ready_queue.size() + (int)input_queue.size() + (int)output_queue.size();

    // Count the active processes in the system
    if (active) {
        cur_in_sys++;
    }
    if (input_active) {
        cur_in_sys++;
    }
    if (output_active) {
        cur_in_sys++;
    }

    // Move processes from entry to ready if they have arrived and there is room in the system
    while (!entry_queue.empty() && entry_queue.front()->arrival_time <= sys_timer && cur_in_sys < IN_USE) {
        Process* p = entry_queue.front();
        entry_queue.pop_front();
        ready_queue.push_back(p);
        cur_in_sys++;
        cout << "Time " << sys_timer << ": " << p->process_name << " moved to Ready" << endl;
    }
}

/*********************************************************************************
* dispatch_cpu
*   Picks the next thing from the ready queue and puts it on the cpu 
*   to start running.
*
* @param none
*********************************************************************************/ 
void dispatch_cpu() {
    // if the cpu is idle and there is something in the ready queue, dispatch it to the cpu
    if (active == nullptr && !ready_queue.empty()) {
        // get the next process from the ready queue
        Process* next_p = ready_queue.front();
        
        // if the process has no more history, remove it from the ready queue and handle termination or skip
        if (next_p->history.empty() || next_p->history_index >= (int)next_p->history.size()) {
            ready_queue.pop_front();
            // Handle termination or skip
            return;
        }

        // dispatch the process to the CPU
        active = next_p;
        ready_queue.pop_front();
        active->cpu_timer = active->history[active->history_index].second;
        quantum_counter = 0;
    }
}

/*********************************************************************************
* handle_cpu
*   Decrements the timer for the active process and checks if its done
*   or if it needs to switch out for round robin.
*
* @param none
*********************************************************************************/
void handle_cpu() {
    // If there is an active process, decrement its timer and check if it's done or if it needs to switch out for round robin
    if (active != nullptr) {
        active->cpu_timer--;
        active->total_cpu_time++;
        quantum_counter++;

        // If the process is done with its cpu burst, move it to the right queue or terminate it
        if (active->cpu_timer == 0) {
            active->cpu_count++;
            active->history_index++;

            // If there are no more bursts, terminate the process
            if (active->history_index >= (int)active->history.size()) {
                terminate_process(active);
                total_done++;
                active = nullptr;

            } else { // Otherwise move it to the right queue
                char burst_type = active->history[active->history_index].first;

                    // If the burst is an input burst, move the process to the input queue
                    if (burst_type == 'I') {
                        input_queue.push_back(active);
                        } else if (burst_type == 'O') { // If the burst is an output burst, move the process to the output queue
                                   output_queue.push_back(active);
                        }
                active = nullptr;
            }

          // If the burst is a cpu burst, move the process to the ready queue
        } else if (quantum_counter == quantum) { // if the process has used up its quantum, move it to the back of the ready queue
            active->history[active->history_index].second = active->cpu_timer;
            ready_queue.push_back(active);
            active = nullptr;
        }
    } else { // If there is no active process, increment idle time
        idle_ticks++;
    }
}

/*********************************************************************************
* handle_input
*   Handles the input device and moves proccess back to ready when 
*   the burst is finished.
*
* @param none
*********************************************************************************/
void handle_input() {
    // If there is no active process on the input device and there is something in the input queue, move it to the input device
    if (input_active == nullptr && !input_queue.empty()) {
        input_active = input_queue.front();
        input_queue.pop_front();
        input_active->input_timer = input_active->history[input_active->history_index].second;
    }
    // If there is an active process on the input device, decrement its timer and check if it's done
    if (input_active) { 
        input_active->input_timer--;
        input_active->total_input_time++;
        if (input_active->input_timer == 0) { // If the input timer is 0, the input burst is done
            input_active->input_count++;
            input_active->history_index++;
            ready_queue.push_back(input_active);
            input_active = nullptr;
        }
    }
}

/*********************************************************************************
* handle_output
*   Handles the output device and moves proccess back to ready when 
*   the burst is finished.
*
* @param none
*********************************************************************************/
void handle_output() {
    // If there is no active process on the output device and there is something in the output queue, move it to the output device
    if (output_active == nullptr && !output_queue.empty()) {
        output_active = output_queue.front();
        output_queue.pop_front();
        output_active->output_timer = output_active->history[output_active->history_index].second;
    }
    // If there is an active process on the output device, decrement its timer and check if it's done
    if (output_active) {
        output_active->output_timer--;
        output_active->total_output_time++;
        if (output_active->output_timer == 0) {
            output_active->output_count++;
            output_active->history_index++;
            ready_queue.push_back(output_active);
            output_active = nullptr;
        }
    }
}

/*********************************************************************************
* update_waiting_time
*   Adds to the wait timer for every proccess stuck in a queue.
*
* @param none
*********************************************************************************/
void update_waiting_time() {
    // Add to the waiting time for each process in the queues
    for (int i = 0; i < (int)ready_queue.size(); i++){
        ready_queue[i]->waiting_time++;
    }
    for (int i = 0; i < (int)input_queue.size(); i++) {
        input_queue[i]->waiting_time++;
    } 
    for (int i = 0; i < (int)output_queue.size(); i++) {
        output_queue[i]->waiting_time++;
    }
}

/*********************************************************************************
* print_state
* Prints out the current state of the system.
*
* @param t - the current time unit
*********************************************************************************/
void print_state(int t) {
    // Print the current state of the system
    cout << "\n--- Time " << t << " ---" << endl;
    cout << "Active: " << (active ? to_string(active->process_id) : "None") << endl;
    cout << "IActive: " << (input_active ? to_string(input_active->process_id) : "None") << endl;
    cout << "OActive: " << (output_active ? to_string(output_active->process_id) : "None") << endl;

    // Print the queues
    cout << "Entry Queue: ";
    if (entry_queue.empty()) cout << "Empty";
    for (size_t i = 0; i < entry_queue.size(); i++) {
        cout << entry_queue[i]->process_id << " ";
    } 
    cout << endl;
    
    // Ready Queue
    cout << "Ready Queue: ";
    if (ready_queue.empty()) {
        cout << "[Empty]";
    } 
    for (size_t i = 0; i < ready_queue.size(); i++) {
        cout << ready_queue[i]->process_id << " ";
    }
    cout << endl;

    // Input Queue
    cout << "Input Queue: ";
    if (input_queue.empty()) {
        cout << "[Empty]";
    }
    for (size_t i = 0; i < input_queue.size(); i++) {
        cout << input_queue[i]->process_id << " ";
    }
    cout << endl;

    // Output Queue
    cout << "Output Queue: ";
    if (output_queue.empty()) cout << "[Empty]";
    for (size_t i = 0; i < output_queue.size(); i++) {
        cout << output_queue[i]->process_id << " ";
    }
    cout << endl;
}
/*********************************************************************************
 * terminate_process
 *   Prints out the stats for a process that is terminating and cleans up memory.
 * 
 * @param p - the process that is terminating
 *********************************************************************************/
void terminate_process(Process* p) {
    // Print the stats for the process that is terminating
    cout << "Time " << sys_timer << ": " << p->process_name << " terminated" << endl;
    cout << "  - Process ID: " << p->process_id << endl;
    cout << "  - CPU Bursts: " << p->cpu_count << endl;
    cout << "  - Input Bursts: " << p->input_count << endl;
    cout << "  - Output Bursts: " << p->output_count << endl;
    cout << "  - Time in CPU: " << p->total_cpu_time << endl;
    cout << "  - Time in Input: " << p->total_input_time << endl;
    cout << "  - Time in Output: " << p->total_output_time << endl;
    cout << "  - Time Waiting: " << p->waiting_time << endl;

    cout << endl;

    // Add the process's times to the total times
    total_waiting_time += p->waiting_time;
    total_cpu_time += p->total_cpu_time;
    total_input_time += p->total_input_time;
    total_output_time += p->total_output_time;

    delete p; // Clean up the memory allocated
}