/*******************************************************************
* CSCI 480 - Assignment 4
* CPU Scheduling Simulation
* Cyrus Johnson | Z2006481
*
* this program simulates how an OS handles cpu scheduling.
* it uses a FCFS algorythm and also supports Round Robin 
* for the extra credit part.
*
* Extra Credit Included   
*******************************************************************/
// libraries
#include <iostream>
#include <vector>
#include <deque> 
#include <string>
#include <fstream>

using namespace std;

/* Constants */
#define MAX_TIME 500
#define IN_USE 5
#define HOW_OFTEN 25

/***************************************************
* Process 
* Process structure to hold process information
***************************************************/
struct Process {
    string process_name;
    int process_id;
    int arrival_time;

    vector<pair<string, int>> history; 

    int sub; // index for history vector

    int cpu_timer;
    int input_timer;
    int output_timer;

    int total_cpu_time;
    int total_input_time;
    int total_output_time;

    int cpu_count;
    int input_count;
    int output_count;

    int waiting_time;
};

/* Global Queues */
deque<Process*> entry_queue;
deque<Process*> ready_queue;
deque<Process*> input_queue;
deque<Process*> output_queue;

/* Global Pointers */
Process* active = nullptr;
Process* i_active = nullptr;
Process* o_active = nullptr;

// system globals
int sys_timer = 0; 
int quantum = 5; 
int q_count = 0;
int idle_ticks = 0;
int total_done = 0;

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
* Function to read the input file and populate the entry queue with processes
* * @param filename - The name of the input file to read
*********************************************************************************/
void read_input_file(string filename) {
    // open the file
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "couldnt open the file!" << endl;
        exit(1);
    }

    // read the file and create processes
    string name;
    int arrival;
    int next_pid = 101;

    // read until we hit STOPHERE
    while (infile >> name && name != "STOPHERE") {
        if (!(infile >> arrival)) break; 

        // create a new process and fill in the details
        Process* p = new Process();
        p->process_name = name;
        p->arrival_time = arrival;
        p->process_id = next_pid++;
        p->sub = 0;

        string type;
        int val;
        
        // read the history until we hit N
        while (infile >> type && type != "N") {
            infile >> val;
            p->history.push_back(make_pair(type, val));
        }
        
        //
        infile >> val; 

        // add to entry queue if it has a history, otherwise clean up memory
        if (!p->history.empty()) {
            entry_queue.push_back(p);
        } else {
            delete p; 
        }
    }
    // close the file
    infile.close();
}

/*********************************************************************************
* main 
* Loop represents the scheduler in the OS. Timer initialized to '0' and
* increments until the timer reaches MAX_TIME or there are no more processes
* left.
*
* @param argc - The number of command line arguments
* @param argv - The array of command line arguments
*********************************************************************************/
int main (int argc, char* argv[]){
    // check for the file argument
    if (argc < 2) {
        cout << "Usage: ./program <file> [quantum]" << endl;
        return 1;
    }

    // rr extra credit
    if (argc == 3) {
        quantum = stoi(argv[2]);
    }

    // read the input file and populate the entry queue
    read_input_file(argv[1]);
    run_simulation();

    // print final stats
    cout << "\n--- Final Stats ---" << endl;
    cout << "Final Timer: " << sys_timer << endl;
    cout << "Proccesses Termimated: " << total_done << endl;
    cout << "Idle Ticks: " << idle_ticks << endl;

    return 0;
}

/*********************************************************************************
* run_simulation
* this is the main loop that runs the hole thing until max time or we
* run out of proccesses to run.
* @param none
*********************************************************************************/
void run_simulation() {
    // loop until max time or we run out of proccesses to run
    while (sys_timer < MAX_TIME) {
        check_arrivals();
        handle_cpu();
        handle_input();
        handle_output();
        dispatch_cpu();
        update_waiting_time();

        // print the state every HOW_OFTEN time units
        if (sys_timer % HOW_OFTEN == 0) {
            print_state(sys_timer);
        }

        // increment the timer
        sys_timer++;

        // stop if nothing left to do
        if (entry_queue.empty() && ready_queue.empty() && input_queue.empty() && 
            output_queue.empty() && !active && !i_active && !o_active) {
            break;
        }
    }
}

/*********************************************************************************
* check_arrivals
* checks if any new proccess arrived in the entry q and moves them to ready
* if there is room in the sys.
* @param none
*********************************************************************************/
void check_arrivals() {
    // check the entry queue for new arrivals and move them to ready if there is room in the system
    int cur_in_sys = (int)ready_queue.size() + (int)input_queue.size() + (int)output_queue.size();
    // count the active processes in the system
    if (active) {
        cur_in_sys++;
    } 
    if (i_active) {
        cur_in_sys++;
    }
    if (o_active) {
        cur_in_sys++;
    }

    // move processes from entry to ready if they have arrived and there is room in the system
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
* picks the next thing from the ready queue and puts it on the cpu 
* to start runing.
* @param none
*********************************************************************************/ 
void dispatch_cpu() {
    // if the cpu is idle and there is something in the ready queue, dispatch it to the cpu
    if (active == nullptr && !ready_queue.empty()) {
        // get the next process from the ready queue
        Process* next_p = ready_queue.front();
        
        // if the process has no more history, remove it from the ready queue and handle termination or skip
        if (next_p->history.empty() || next_p->sub >= (int)next_p->history.size()) {
            ready_queue.pop_front();
            // Handle termination or skip
            return;
        }

        // dispatch the process to the CPU
        active = next_p;
        ready_queue.pop_front();
        active->cpu_timer = active->history[active->sub].second;
        q_count = 0;
    }
}

/*********************************************************************************
* handle_cpu
* decrements the timer for the active proccess and checks if its done
* or if it needs to switch out for round robin.
* @param none
*********************************************************************************/
void handle_cpu() {
    // if there is an active process, decrement its timer and check if it's done or if it needs to switch out for round robin
    if (active != nullptr) {
        active->cpu_timer--;
        active->total_cpu_time++;
        q_count++;

        // if the process is done with its cpu burst, move it to the right queue or terminate it
        if (active->cpu_timer == 0) {
            active->cpu_count++;
            active->sub++;

            // if there are no more bursts, terminate the process
            if (active->sub >= (int)active->history.size()) {
                terminate_process(active);
                total_done++;
                active = nullptr;

            } else { // otherwise move it to the right queue
                if (active->history[active->sub].first == "I") input_queue.push_back(active);
                else output_queue.push_back(active);
                active = nullptr;
            }

        } else if (q_count == quantum) { // if the process has used up its quantum, move it to the back of the ready queue
            active->history[active->sub].second = active->cpu_timer;
            ready_queue.push_back(active);
            active = nullptr;
        }
    } else { // if there is no active process, increment idle time
        idle_ticks++;
    }
}

/*********************************************************************************
* handle_input
* handles the input device and moves proccess back to ready when 
* the burst is finished.
* @param none
*********************************************************************************/
void handle_input() {
    // if there is no active process on the input device and there is something in the input queue, move it to the input device
    if (i_active == nullptr && !input_queue.empty()) {
        i_active = input_queue.front();
        input_queue.pop_front();
        i_active->input_timer = i_active->history[i_active->sub].second;
    }
    // if there is an active process on the input device, decrement its timer and check if it's done
    if (i_active) { 
        i_active->input_timer--;
        i_active->total_input_time++;
        if (i_active->input_timer == 0) {
            i_active->input_count++;
            i_active->sub++;
            ready_queue.push_back(i_active);
            i_active = nullptr;
        }
    }
}

/*********************************************************************************
* handle_output
* does the same as input but for the output device.
* @param none
*********************************************************************************/
void handle_output() {
    //
    if (o_active == nullptr && !output_queue.empty()) {
        o_active = output_queue.front();
        output_queue.pop_front();
        o_active->output_timer = o_active->history[o_active->sub].second;
    }
    // if there is an active process on the output device, decrement its timer and check if it's done
    if (o_active) {
        o_active->output_timer--;
        o_active->total_output_time++;
        if (o_active->output_timer == 0) {
            o_active->output_count++;
            o_active->sub++;
            ready_queue.push_back(o_active);
            o_active = nullptr;
        }
    }
}

/*********************************************************************************
* update_waiting_time
* adds to the wait timer for every proccess stuck in a queue.
* @param none
*********************************************************************************/
void update_waiting_time() {
    // jus loop and add time
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
* prints out what is happening right now for the assignemnt output.
* @param t - the current time unit recieved
*********************************************************************************/
void print_state(int t) {
    // print the current state of the system
    cout << "\n--- Time " << t << " ---" << endl;
    cout << "Active: " << (active ? to_string(active->process_id) : "None") << endl;
    cout << "IActive: " << (i_active ? to_string(i_active->process_id) : "None") << endl;
    cout << "OActive: " << (o_active ? to_string(o_active->process_id) : "None") << endl;

    // print the queues
    cout << "Entry Q: ";
    if (entry_queue.empty()) cout << "Empty";
    for (size_t i = 0; i < entry_queue.size(); i++) {
        cout << entry_queue[i]->process_id << " ";
    } 
    cout << endl;

    cout << "Ready Q: ";
    if (ready_queue.empty()) {
        cout << "Empty!!";
    } 
    for (size_t i = 0; i < ready_queue.size(); i++) {
        cout << ready_queue[i]->process_id << " ";
    }
    cout << endl;

    cout << "Input Q: ";
    if (input_queue.empty()) {
        cout << "Empty!!";
    }
    for (size_t i = 0; i < input_queue.size(); i++) {
        cout << input_queue[i]->process_id << " ";
    }
    cout << endl;

    cout << "Output Q: ";
    if (output_queue.empty()) cout << "Empty!!";
    for (size_t i = 0; i < output_queue.size(); i++) {
        cout << output_queue[i]->process_id << " ";
    }
    cout << endl;
}
/*********************************************************************************
 * terminate_process
 * prints out the stats for a process that is terminating and cleans up memory.
 * @param p - the process that is terminating
 *********************************************************************************/
void terminate_process(Process* p) {
    // print the stats for the process that is terminating
    cout << "Time " << sys_timer << ": " << p->process_name << " terminated" << endl;
    cout << "  - Process ID: " << p->process_id << endl;
    cout << "  - CPU Bursts: " << p->cpu_count << endl;           // [cite: 102]
    cout << "  - Input Bursts: " << p->input_count << endl;       // [cite: 103]
    cout << "  - Output Bursts: " << p->output_count << endl;     // [cite: 104]
    cout << "  - Time in CPU: " << p->total_cpu_time << endl;     // [cite: 105]
    cout << "  - Time in Input: " << p->total_input_time << endl; // [cite: 106]
    cout << "  - Time in Output: " << p->total_output_time << endl;// [cite: 107]
    cout << "  - Time Waiting: " << p->waiting_time << endl;      // [cite: 108]
    cout << endl;

    delete p; // Clean up the memory allocated
}