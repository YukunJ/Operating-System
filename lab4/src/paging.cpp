//  NYU Courant Institute
//  paging.cpp
//  Operating System Spring 2020
//  Lab 4 : Demaning pager
//  Algorithm: FIFO, LRU, RANDOM
//
//  Created by Yukun, Jiang on 4/14/20.
//  Copyright © 2020 Yukun, Jiang. All rights reserved.
//

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <climits>
#define QUANTUM 3
using namespace std;

long get_random(ifstream& ifs);

/*
 class Process:
		simulate the running of a process
		it records all the necessary information about the process
		in the public scope for easy reference
*/
class Process {
public:
	int S;
	int N;
	double A;
	double B;
	double C;
	int id;
	int page_fault;
	int next_ref;
	int rest_time;
	int evict_time;
};

/*
 class Frame: (to be nested in class FrameTable)
		simulate a frame with process and page in it.
		contains other information like the load_time, last_reference_time, etc.
*/
class Frame {
public:
	Frame(int process_id = -1, int page = -1, int inTime = -1, int lastRef = -1) :
				process_id(process_id), page(page), inTime(inTime), lastRef(lastRef) {}
	int process_id;
	int page;
	int inTime;
	int lastRef;
};

/*
 class FrameTable: (contain Class Frame in an array)
		The only frameTable in the system, simulating the physical memory
		contains array several pages depending on machine size and page size
*/
class FrameTable {
public:
	FrameTable(int _size, string _strategy) {
		table = new Frame[_size];
		size = _size;
		for (int i = 0; i < size; i ++) {
			table[i].process_id = -1;
			table[i].page = -1;
			table[i].inTime = -1;
			table[i].lastRef = -1;
		}
		if (_strategy == "fifo")
			strategy = 0;
		else if (_strategy == "random")
			strategy = 1;
		else if (_strategy == "lru")
			strategy = 2;
		else
			cout << "replacement strategy wrong. Please check again!" << endl;
	}
	
	~FrameTable() {delete[] table;}
	
	Frame& operator[](int index) {
		return table[index];
	}
	
	int find_free_frame() {
		// search for a free frame, from highest index to lowest
		for (int i = size - 1; i >= 0; i--) {
			if (table[i].process_id == -1)
				return i;
		}
		return -1;
	}
	
	int find_ref(int process_id, int page_num, int time) {
		// linearly search the frameTable to see
		// if a specific page for a process is resident in it
		for (int i = 0; i < size; i++) {
			if (table[i].process_id == process_id && table[i].page == page_num) {
				table[i].lastRef = time;
				return i;
			}
		}
		return -1;
	}
	
	int evict(ifstream& ifs) {
		// evict a page in the frameTable depending on the Replacement Policy
		int to_evict;
		if (strategy == 0) {
			//FIFO : check for the earliest entered page
			int evict_index = -1;
			int evict_inTime = 99999;
			for (int i = 0; i < size; i++) {
				if (table[i].inTime < evict_inTime) {
					evict_index = i;
					evict_inTime = table[i].inTime;
				}
			}
			if (evict_index == -1) {cout << "Error，evict_index = -1 in FIFO, Check again\n";}
			to_evict = evict_index;
		}
		else if (strategy == 1) {
			// Random : pick a random number and do module
			to_evict = (int) (get_random(ifs) % size);
		}
		else if (strategy == 2) {
			// LRU : pick the least recently used pages
			int evict_index = -1;
			int evict_lastRef = 99999;
			for (int i = 0; i < size; i++) {
				if (table[i].lastRef < evict_lastRef) {
					evict_index = i;
					evict_lastRef = table[i].lastRef;
				}
			}
		if (evict_index == -1) {cout << "Error，evict_index = -1 in LRU, Check again\n";}
		to_evict = evict_index;
		}
		return to_evict;
	}
	
	Frame* table;
	int size;
	int strategy;
};

void open_random(ifstream& ifs) {
	// open the random_number txt files
	ifs.open("random-numbers.txt");
	if (!ifs) {
		cerr << "Could not open file 'random-numbers.txt'. Please check again." << endl;
		exit(1);
	}
}

void close_random(ifstream& ifs) {
	// for safety, close the file stream
	ifs.close();
}

long get_random(ifstream& ifs) {
	// get the next random number,
	// notice the type is long for portability
	long next;
	ifs >> next;
	return next;
}

void show_sys(int M, int P, int S, int J, int N, string R) {
	// moduleized print function displaying information about the system
	cout << "The machine size is " << M << "." << endl;
	cout << "The page size is " << P << "." << endl;
	cout << "The process size is " << S << "." << endl;
	cout << "The job mix number is " << J << "." << endl;
	cout << "The number of reference per process is " << N << "." << endl;
	cout << "The replacement algorithm is " << R << "." << endl;
}

void process_table_init(int S, int J, int N, Process*& processTable, int& process_num) {
	// hand-made initialization of the processTable
	if (J == 1) {
		process_num = 1;
		processTable = new Process[1];
		processTable[0].S = S;
		processTable[0].N = N;
		processTable[0].A = 1;
		processTable[0].B = 0;
		processTable[0].C = 0;
		processTable[0].id = 0;
		processTable[0].next_ref = 0;
		processTable[0].rest_time = 0;
		processTable[0].evict_time = 0;
		processTable[0].page_fault = 0;
	}
	else if (J == 2) {
		process_num = 4;
		processTable = new Process[4];
		processTable[0].S = S;
		processTable[0].N = N;
		processTable[0].A = 1;
		processTable[0].B = 0;
		processTable[0].C = 0;
		processTable[0].id = 0;
		processTable[0].next_ref = 0;
		processTable[0].rest_time = 0;
		processTable[0].evict_time = 0;
		processTable[0].page_fault = 0;
		
		processTable[1].S = S;
		processTable[1].N = N;
		processTable[1].A = 1;
		processTable[1].B = 0;
		processTable[1].C = 0;
		processTable[1].id = 1;
		processTable[1].next_ref = 0;
		processTable[1].rest_time = 0;
		processTable[1].evict_time = 0;
		processTable[1].page_fault = 0;
		
		processTable[2].S = S;
		processTable[2].N = N;
		processTable[2].A = 1;
		processTable[2].B = 0;
		processTable[2].C = 0;
		processTable[2].id = 2;
		processTable[2].next_ref = 0;
		processTable[2].rest_time = 0;
		processTable[2].evict_time = 0;
		processTable[2].page_fault = 0;
		
		processTable[3].S = S;
		processTable[3].N = N;
		processTable[3].A = 1;
		processTable[3].B = 0;
		processTable[3].C = 0;
		processTable[3].id = 3;
		processTable[3].next_ref = 0;
		processTable[3].rest_time = 0;
		processTable[3].evict_time = 0;
		processTable[3].page_fault = 0;
	}
	else if (J == 3) {
		process_num = 4;
		processTable = new Process[4];
		processTable[0].S = S;
		processTable[0].N = N;
		processTable[0].A = 0;
		processTable[0].B = 0;
		processTable[0].C = 0;
		processTable[0].id = 0;
		processTable[0].next_ref = 0;
		processTable[0].rest_time = 0;
		processTable[0].evict_time = 0;
		processTable[0].page_fault = 0;
		
		processTable[1].S = S;
		processTable[1].N = N;
		processTable[1].A = 0;
		processTable[1].B = 0;
		processTable[1].C = 0;
		processTable[1].id = 1;
		processTable[1].next_ref = 0;
		processTable[1].rest_time = 0;
		processTable[1].evict_time = 0;
		processTable[1].page_fault = 0;
		
		processTable[2].S = S;
		processTable[2].N = N;
		processTable[2].A = 0;
		processTable[2].B = 0;
		processTable[2].C = 0;
		processTable[2].id = 2;
		processTable[2].next_ref = 0;
		processTable[2].rest_time = 0;
		processTable[2].evict_time = 0;
		processTable[2].page_fault = 0;
		
		processTable[3].S = S;
		processTable[3].N = N;
		processTable[3].A = 0;
		processTable[3].B = 0;
		processTable[3].C = 0;
		processTable[3].id = 3;
		processTable[3].next_ref = 0;
		processTable[3].rest_time = 0;
		processTable[3].evict_time = 0;
		processTable[3].page_fault = 0;
	}
	else if (J == 4) {
		process_num = 4;
		processTable = new Process[4];
		processTable[0].S = S;
		processTable[0].N = N;
		processTable[0].A = 0.75;
		processTable[0].B = 0.25;
		processTable[0].C = 0;
		processTable[0].id = 0;
		processTable[0].next_ref = 0;
		processTable[0].rest_time = 0;
		processTable[0].evict_time = 0;
		processTable[0].page_fault = 0;
		
		processTable[1].S = S;
		processTable[1].N = N;
		processTable[1].A = 0.75;
		processTable[1].B = 0;
		processTable[1].C = 0.25;
		processTable[1].id = 1;
		processTable[1].next_ref = 0;
		processTable[1].rest_time = 0;
		processTable[1].evict_time = 0;
		processTable[1].page_fault = 0;
		
		processTable[2].S = S;
		processTable[2].N = N;
		processTable[2].A = 0.75;
		processTable[2].B = 0.125;
		processTable[2].C = 0.125;
		processTable[2].id = 2;
		processTable[2].next_ref = 0;
		processTable[2].rest_time = 0;
		processTable[2].evict_time = 0;
		processTable[2].page_fault = 0;
		
		processTable[3].S = S;
		processTable[3].N = N;
		processTable[3].A = 0.5;
		processTable[3].B = 0.125;
		processTable[3].C = 0.125;
		processTable[3].id = 3;
		processTable[3].next_ref = 0;
		processTable[3].rest_time = 0;
		processTable[3].evict_time = 0;
		processTable[3].page_fault = 0;
	}
}

int main(int argc, char* argv[]) {
	// Read Command line input
	// Do the initialization and variable setup
	ifstream ifs;
	open_random(ifs);
	int M = atoi(argv[1]);
	int P = atoi(argv[2]);
	int S = atoi(argv[3]);
	int J = atoi(argv[4]);
	int N = atoi(argv[5]);
	int debug = 0;
	string R = argv[6];
	show_sys(M,P,S,J,N,R);
	if (argc == 8) {
		debug = atoi(argv[7]);
		cout << "The level of debugging output is " << debug << "." << endl;
	}
	cout << endl;
	int process_num, reference_num = 0, flag = 0;
	Process* processTable;
	FrameTable frameTable(M/P, R);
	process_table_init(S, J, N, processTable, process_num);
	
	// Set the initial reference by specification
	for (int i = 0; i < process_num; i++)
		processTable[i].next_ref = (111 * (i+1)) % processTable[i].S;
	while (1) {
		for (int id = 0; id < process_num; id++) {
			for (int ref = 0; ref < QUANTUM; ref++) {
				// if already finish all the reference, just continue
				// don't disturb the random number generator
				if (processTable[id].N == 0)
					continue;
				reference_num += 1;
				// simulate this reference for this process
				int ref_loc = processTable[id].next_ref;
				int page_loc = ref_loc / P;
				int ref_to_find = frameTable.find_ref(id, page_loc, reference_num);
				if (debug == 1) // debug information print
				cout << id+1 << " reference word " << ref_loc << " (page " << page_loc << ") at time " << reference_num << ":";
				if (ref_to_find != -1 && debug == 1) { // debug information print
					cout << "Hit in frame " << ref_to_find << endl;
				}
				if (ref_to_find == -1) {// didn't find the page looking for, non-resident reference
					processTable[id].page_fault += 1;
					// first try find a free frame to fit in
					int free_frame = frameTable.find_free_frame();
					if (free_frame != -1) {
						if (debug == 1) // debug information print
							cout << "Fault, using free frame " << free_frame << "." << endl;
						frameTable[free_frame].process_id = id;
						frameTable[free_frame].page = page_loc;
						frameTable[free_frame].inTime = reference_num;
						frameTable[free_frame].lastRef = reference_num;
					}
					else {
					// no free frame, then evict a frame and fit in
						int to_evict = frameTable.evict(ifs);
						int evict_id = frameTable[to_evict].process_id;
						if (debug == 1) // debug information print
						cout << "Fault, evicting page " << frameTable[to_evict].page << " of " << frameTable[to_evict].process_id+1 << " from frame " << to_evict << "." << endl;
						processTable[evict_id].rest_time += (reference_num - frameTable[to_evict].inTime);
						processTable[evict_id].evict_time += 1;
						frameTable[to_evict].process_id = id;
						frameTable[to_evict].page = page_loc;
						frameTable[to_evict].inTime = reference_num;
						frameTable[to_evict].lastRef = reference_num;
					}
				}
				processTable[id].N -= 1; // finish one reference for this process
				if (processTable[id].N == 0)
					flag = 1;
				// calculate the next reference for this process
				int last_ref = processTable[id].next_ref;
				double y = get_random(ifs) / (INT_MAX +1.0);
				double A = processTable[id].A;
				double B = processTable[id].B;
				double C = processTable[id].C;
				if (y < A) // case 1 with probability A
					processTable[id].next_ref = (last_ref + 1) % processTable[id].S;
				else if (y < A + B) // case 2 with probability B
					processTable[id].next_ref = (last_ref - 5 + processTable[id].S) % processTable[id].S;
				else if (y < A + B + C) // case 3 with probability C
					processTable[id].next_ref = (last_ref + 4) % processTable[id].S;
				else // case 4 with probability 1-A-B-C
					processTable[id].next_ref = (int)(get_random(ifs) % processTable[id].S);
			}
		}
		if (flag == 1) // all process finish at the same stage
			break;
	}
	// std::cout
	float total_fault = 0.0;
	float total_evict = 0.0;
	float total_residency = 0.0;
	for (int i = 0; i < process_num; i++) {
		total_fault += processTable[i].page_fault;
		total_evict += processTable[i].evict_time;
		total_residency += processTable[i].rest_time;
		if (processTable[i].evict_time == 0) {
			cout << "Process " << i+1 << " had " << processTable[i].page_fault << " faults." << endl;
			cout << "\tWith no evictions, the average residence is undefined." << endl;
		}
		else {
			cout << "Process " << i+1 << " had " << processTable[i].page_fault;
			cout << " faults and " << (float)(processTable[i].rest_time * 1.0)/processTable[i].evict_time;
			cout << " average residency." << endl;
		}
	}
	cout << endl;
	if (total_evict == 0.0) {
		cout << "The total number of faults is " << total_fault << "." << endl;
		cout << "\tWith no evictions, the overall average residence is undefined." << endl;
	}
	else {
		cout << "The total number of faults is " << total_fault;
		cout << " and the overall average residency is " << total_residency / total_evict << "." << endl;
	}
	close_random(ifs);
	delete[] processTable;
	return 0;
}
