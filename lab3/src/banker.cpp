//
//  banker.cpp
//  Operating System Spring 2020
//  Lab 3 : 1. optimistic resource manager
//          2. banker's algorithm of Dijkstra
//
//  Created by Yukun, Jiang on 4/2/20.
//  Copyright © 2020 Yukun, Jiang. All rights reserved.
//

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

/* The nestes class: Instruction
   It essentially extract the information of a line of the input file
	 including the action {initiate, request, release, terminate},
								 delay, resourceType and resourceQuantity
   The class contains a few setters and getters, as its name
                       indicates its functionality
*/
class Instruction {
public:
	Instruction(string givenInstruction) {
		stringstream ss(givenInstruction);
		string temp_instructionType, temp_processRank, temp_delay, temp_resourceType, temp_resourceQuantity;
		ss >> temp_instructionType >> temp_processRank >> temp_delay >> temp_resourceType >> temp_resourceQuantity;
		instructionType = temp_instructionType;
		delay = atoi(temp_delay.c_str());
		resourceType = atoi(temp_resourceType.c_str());
		resourceQuantity = atoi(temp_resourceQuantity.c_str());
	}
	Instruction(const Instruction& rhs) {
		instructionType = rhs.getInstructionType();
		delay = rhs.getDelay();
		resourceType = rhs.getResourceType();
		resourceQuantity = rhs.getResourceQuantity();
	}
	Instruction& operator=(const Instruction& rhs) {
		if (this == &rhs)
			return *this;
		instructionType = rhs.getInstructionType();
		delay = rhs.getDelay();
		resourceType = rhs.getResourceType();
		resourceQuantity = rhs.getResourceQuantity();
		return (*this);
	}
	void decreaseDelay() {delay -= 1;}
	string getInstructionType() const {return instructionType;}
	int getDelay() const {return delay;}
	int getResourceType() const {return resourceType;}
	int getResourceQuantity() const {return resourceQuantity;}
	friend ostream& operator<<(ostream& os, const Instruction& rhs) {
		os << rhs.getInstructionType() << " ";
		os << rhs.getDelay() << " " << rhs.getResourceType() << " " << rhs.getResourceQuantity();
		os << endl;
		return os;
	}
private:
	string instructionType;
	int delay;
	int resourceType;
	int resourceQuantity;
};

/*
 The main class object Process ( vector<Instruction> nested inside)
 It simulates the running states of a process in the algorithm
 It essentially record all the information about a process,
 including its rank, how far it goes in all its instructions, its initial claim,
           the allocated resource so far, whether finished or aborted, etc.
 The class contains a few setters and getters, as its name
										 indicates its functionality
*/
class Process {
public:
	Process(int initialRank = 0, int initialIndex = 0, int initialWait = 0, int initialTerminate = 0, int initialAbort = 0, int initialInitiated = 0):
	rank(initialRank), instructionIndex(initialIndex), wait(initialWait), terminate(initialTerminate), abort(initialAbort),initiated(initialInitiated) {}
	Process(const Process& rhs) {
		rank = rhs.getRank();
		instructionIndex = rhs.getInstructionIndex();
		wait = rhs.getWait();
		terminate = rhs.getFinish();
		abort = rhs.getAbort();
		instructionTable.clear();
		resourceTable.clear();
		for (Instruction instruction: rhs.getInstructionTable())
			instructionTable.push_back(instruction);
		for (int resourceCount: rhs.getResourceTable() )
			resourceTable.push_back(resourceCount);
		claimTable = rhs.getClaimTable();
	}
	Process& operator=(const Process& rhs) {
		if (this == &rhs)
			return *this;
		rank = rhs.getRank();
		instructionIndex = rhs.getInstructionIndex();
		wait = rhs.getWait();
		terminate = rhs.getFinish();
		abort = rhs.getAbort();
		instructionTable.clear();
		resourceTable.clear();
		for (Instruction instruction: rhs.getInstructionTable())
			instructionTable.push_back(instruction);
		for (int resourceCount: rhs.getResourceTable() )
			resourceTable.push_back(resourceCount);
		claimTable = rhs.getClaimTable();
		return *this;
	}
	vector<Instruction> getInstructionTable() const {return instructionTable;}
	vector<int> getResourceTable() const {return resourceTable;}
	void setRank(int newRank) {rank = newRank;}
	void setResourceTable(int resourceNum) {vector<int> table(resourceNum+1 ,0); resourceTable = table;}
	void setClaimTable(int resourceNum) {vector<int> table(resourceNum+1, 0); claimTable = table;}
	void addResource(int resourceType, int resourceQuantity) {resourceTable[resourceType] += resourceQuantity;}
	void decreaseResource(int resourceType, int resourceQuantity) {resourceTable[resourceType] -= resourceQuantity;}
	vector<int> getClaimTable() const {return claimTable;}
	int getInitiated() const {return initiated;}
	void setInitiated() {initiated = 1;}
	void incrementIndex() {instructionIndex += 1;}
	void setFinish(int finishTime) { terminate = finishTime;}
	void decreaseDelay() {instructionTable[instructionIndex].decreaseDelay();}
	void addWait() {wait += 1;}
	void setAbort() {abort = 1;}
	void setClaim(int resourceType, int resourceQuantity) {claimTable[resourceType] = resourceQuantity;}
	int getInstructionIndex() const {return instructionIndex;}
	bool isAbort() {return abort==1 ;}
	int getRank() const {return rank;}
	int getAbort() const {return abort;}
	int getNextDelay() const {return instructionTable[instructionIndex].getDelay();}
	int getNextNextDelay() const {return instructionTable[instructionIndex+1].getDelay();}
	int getWait() const {return wait;}
	int getFinish() const {return terminate;}
	string getNextInstructionType() const {return instructionTable[instructionIndex].getInstructionType();}
	string getNextNextInstructionType() const {return instructionTable[instructionIndex+1].getInstructionType();}
	Instruction getNextInstruction() const {return instructionTable[instructionIndex];}
	void addInstruction(string givenInstruction) {
		instructionTable.push_back(Instruction(givenInstruction));
	}
	friend ostream& operator<<(ostream& os, const Process& rhs) {
		if (rhs.getRank() == 0) {
			os << "Process with Rank 0 is junk process, no display."<< endl;
			return os;
		}
		os << "Process rank: " << rhs.getRank() << endl;
		for (Instruction& each: rhs.getInstructionTable())
			os << each;
		os << endl;
		return os;
	}
private:
	int rank;
	int instructionIndex;
	int wait;
	int terminate;
	int abort;
	int initiated;
	vector<Instruction> instructionTable;
	vector<int> resourceTable;
	vector<int> claimTable;
};

/*
 Function : findLowestIndex
 used in Optimistic Resource Allocater Algorithm
 to find the lowest rank process in the blocking list
 in order to "abort" it to release resources to solve deadlock
*/
int findLowestIndex(vector<Process>& blocking) {
	int miniIndex = blocking[0].getRank();
	for (Process& process: blocking) {
		if (process.getRank() < miniIndex)
			miniIndex = process.getRank();
	}
	return miniIndex;
}

/*
 Function : openFile
 used to open the input file
 if unsuccessful, print error message to std::cout and exit with code 1
*/
void openFile(char* fileName, ifstream& ifs) {
	ifs.open(fileName);
	if (!ifs) {
		cout << "Failure to open the file :<" << fileName << ">." << endl;
		exit(1);
	}
}

/*
 Function : findProcess
 given a vector of Process and a specified rank,
 used to find the index of the process within the vector with that specified rank
 return the index found, or -1 if not found
*/
int findProcess(vector<Process>& processVector, int rank_toFind) {
	for (int i = 0; i < processVector.size(); i++){
		if (processVector[i].getRank() == rank_toFind)
			return i;
	}
	return -1;
}

/*
 Function: initialize
 used in the Optimistic Resource Allocator to do initialization
 once a process has finished all its initiaition ( i.e. if next instruction is "request")
 forward it to the running list
*/
void initialize(vector<Process>& processVector,vector<Process>& running, vector<Process>& uninitiated,
								vector<int> resourceVector, int processNum, int resourceNum, int& initial_to_running, int optimistic = 0) {
	if (optimistic == 0) {
		for (int i = 1; i <= processNum; i++) {
			if (processVector[i].getNextInstructionType() == "initiate")
				processVector[i].incrementIndex();
			if (processVector[i].getNextInstructionType() != "initiate" && processVector[i].getInitiated() == 0) {
				processVector[i].setInitiated();
				running.push_back(processVector[i]);
				initial_to_running += 1;
			}
		}
	}
}

/*
 function : banker_initialize
 used in the banker's algorithm to do initialization
 we record the process's claim and check if it exceeds the avilable resources in system
 if exceed, abort it and print out a message to std::cout
*/
void banker_initialize(vector<Process>& processVector, vector<Process>& running, vector<int>& originalAvailable, int processNum, int resourceNum, int& initial_to_running, int& aborted, int cycle) {
	for (int i = 1; i <= processNum; i++) {
		if (processVector[i].getNextInstructionType() == "initiate" && processVector[i].getAbort() != 1) {
			Instruction initial_claim = processVector[i].getNextInstruction();
			int resourceType = initial_claim.getResourceType();
			int resourceQuantity = initial_claim.getResourceQuantity();
			if (resourceQuantity > originalAvailable[resourceType]) {// claim too much, abort
				cout << "Banker aborts task " << i << " in initialization:" << endl;
				cout << "     claim for resource " << resourceType << " (";
				cout << resourceQuantity << ") exceeds number of units present (";
				cout << originalAvailable[resourceType] << ")" << endl;
				aborted += 1;
				processVector[i].setAbort();
			}
			else {
				processVector[i].setClaim(resourceType, resourceQuantity);
				processVector[i].incrementIndex();
			}
			if (processVector[i].getNextInstructionType() != "initiate" && processVector[i].getInitiated() == 0) {
				processVector[i].setInitiated();
				running.push_back(processVector[i]);
				initial_to_running += 1;
			}
		}
	}
}

/*
 function : checkDeadlock
 used in Optimistic Resource Allocator
 the condition for entering this function is (len(running) == 0 && len(blocking) > 0)
 if any process in blocking list could bd satisified, return false
 else, it's deadlocked, return true
*/
bool checkDeadlock(vector<Process>& blocking, vector<int>& tempResourceVector) {
	if (blocking.size() == 0)
		return false;
	bool deadlock = true;
	for (Process& process: blocking) {
		Instruction newrequest = process.getNextInstruction();
		if (tempResourceVector[newrequest.getResourceType()] >= newrequest.getResourceQuantity()) {
			deadlock = false;
		}
	}
	return deadlock;
}

/*
 function : release
 used in both algorithms for the instruction {release}
 it add the released resource to the <vector>int availNextCycle
 and subtract the resource released from the process's allocated resource vector
*/
bool release(Process& process, vector<int>& availNextCycle) {
	if (process.getNextInstructionType() != "release") {
		cout << "Release is not an Release, check again!" << endl;
		return false;
	}
	Instruction newrelease = process.getNextInstruction();
	availNextCycle[newrelease.getResourceType()] += newrelease.getResourceQuantity();
	process.decreaseResource(newrelease.getResourceType(), newrelease.getResourceQuantity());
	process.incrementIndex();
	return true;
}

/*
 function : releaseAbort
 used to release all the resources a process
				hold when it's aborted
 Notice this is different from instruction "release"
			  which only release 1 type of resource
*/
bool releaseAbort(Process& process, vector<int>& availNextCycle,vector<int>& tempResourceVector) {
	vector<int> resourceTable = process.getResourceTable();
	for (int i = 0; i < resourceTable.size(); i++) {
		availNextCycle[i] += resourceTable[i];
		tempResourceVector[i] += resourceTable[i];
	}
	return true;
}

/*
 function : request
 used in Optimistic Resource Allocator
 if a request could be satisfied, then add resource to the process, return true
 else just return false to indicate Failure
*/
bool request(Process& process, vector<int>& resourceVector) {
	if (process.getNextInstructionType() != "request") {
		cout << "Blocking is not an Request, check again!" << endl;
		return false;
	}
	Instruction newrequest = process.getNextInstruction();
	if (resourceVector[newrequest.getResourceType()] >= newrequest.getResourceQuantity()){ // request success
		resourceVector[newrequest.getResourceType()] -= newrequest.getResourceQuantity();
		process.addResource(newrequest.getResourceType(),newrequest.getResourceQuantity());
		process.incrementIndex();
		return true;
	}
	else
		return false;
}

/*
 Function: isSafeState
 used in banker's algorithm to check if a request leads to a safe state
 1. first check if the request exceeds its claim
    if so, abort it and print a message to std::cot
 2. do simulation, assume the request grant, then to see if
    the system is in a safe state (all process could run to completion)
 return 1 if safe state
 return 0 if unsafe state
 return 2 if aborted
*/
int isSafeState(vector<Process>& processVector, vector<Process>& running, vector<Process>& blocking, int processNum, int resourceNum, int rank, Instruction theRequest, vector<int> resourceVector, vector<int>& availNextCycle, int& aborted, int cycle) {
	// do a simulation to see if the current request would lead to a safe state
	int resourceType = theRequest.getResourceType();
	int resourceQuantity = theRequest.getResourceQuantity();
	vector<Process> temp_local_running;
	vector<Process> local_running;
	vector<Process> temp_running;
	vector<Process> temp_blocking;
	for (Process process: running)
		local_running.push_back(process);
	for (Process process: blocking)
		local_running.push_back(process);
	int index = findProcess(local_running, rank);
	int index_running = findProcess(running, rank);
	int index_blocking = findProcess(blocking, rank);
	if (index == -1) // set a guard for debug
		cout << "Problem occurs! Not find the process with rank " << rank << endl;
	// check if it should be aborted if ask exceeds its claim
	vector<int> claim = local_running[index].getClaimTable();
	vector<int> allocated = local_running[index].getResourceTable();
	if (resourceQuantity > claim[resourceType]-allocated[resourceType]) {
		cout << "During cycle " << cycle-1 << "-" << cycle << " of Banker's algorithm" << endl;
		cout << "   Task " << local_running[index].getRank() << "'s request ";
		cout << "exceeds its claim; aborted; ";
		cout << "[";
		for (int i = 1; i <= resourceNum; i++)
			cout << allocated[i] << " ";
		cout << "] ";
		cout << "units available next cycle" << endl;
		aborted += 1;
		for (int i = 1; i <= resourceNum; i++)
			availNextCycle[i] += allocated[i];
		int process_index = findProcess(processVector, local_running[index].getRank());
		processVector[process_index].setAbort();
		// remove it from the running/ blocking list
		if (index_running != -1){ // it's in runninglist
			for (Process process: running) {
				if (index_running != process.getRank())
					temp_running.push_back(process);
			}
			running = temp_running;
		}
		else { // it's in blocking list, but this would not happen actually
			for (Process process: blocking) {
				if (index_blocking != process.getRank())
					temp_blocking.push_back(process);
			}
			blocking = temp_blocking;
		}
		return 2;
	}
	if (resourceVector[resourceType] < resourceQuantity) // not enough available, just return
		return 0;
	// assume banker satisfy the request and give it to process
	local_running[index].addResource(resourceType, resourceQuantity);
	resourceVector[resourceType] -= resourceQuantity;
	// start the simulation to see if safe state
	int finished = 0;
	int to_finish = (int)local_running.size();
	for (int j = 0; j <= local_running.size(); j++) { // at most size() times of iteration
		temp_local_running.clear();
		for (Process process: local_running) { // check each process, is it possible to terminate?
			bool finishable = true;
			vector<int> claim = process.getClaimTable();
			vector<int> allocated = process.getResourceTable();
			for (int k = 1; k <= resourceNum; k++) {
				if (not (resourceVector[k] >= (claim[k]-allocated[k])))
					finishable = false;
				}
			if (finishable) {
				// free its resource and kick out of the running list
				for (int m = 1; m <= resourceNum; m++) {
					resourceVector[m] += allocated[m];
				}
				finished += 1;
			}
			else { // not finishable, wait for next round
				temp_local_running.push_back(process);
			}
		}
		local_running = temp_local_running;
	}
	if (finished == to_finish)
		return 1;
	else
		return 0;
}

/*
 Function : banker
 the main function for running Banker's algorithm
 more details line by line inside
 we use the idea of filtering for list
*/
void banker(char * fileName,int detailed = 0) {
	// do initialization of setting up parameters
	ifstream ifs;
	openFile(fileName,ifs);
	int processNum, resourceNum, resourceCount;
	bool to_break = false;
	ifs >> processNum >> resourceNum;
	vector<Process> processVector(processNum+1);
	vector<int> resourceVector(resourceNum+1);
	vector<int> tempResourceVector;
	vector<Process> uninitiated, running, blocking, terminated;
	vector<Process> tempRunning, tempBlocking;
	for (int i = 1; i <= processNum; i++){
		processVector[i].setRank(i);
		processVector[i].setResourceTable(resourceNum);
		processVector[i].setClaimTable(resourceNum);
	}
	for (int i = 1; i <= resourceNum; i++) {
		ifs >> resourceCount;
		resourceVector[i] = resourceCount;
	}
	while (ifs) {
		string instructionType, processRank, delay, resourceType, resourceQuantity;
		ifs >> instructionType >> processRank >> delay >> resourceType >> resourceQuantity;
		string givenInstruction = instructionType + " " + processRank + " " + delay + " " + resourceType + " " + resourceQuantity;
		int rank_toFind = atoi(processRank.c_str());
		int index = findProcess(processVector, rank_toFind);
		processVector[index].addInstruction(givenInstruction);
	}
	int cycle = 0, finished = 0, aborted = 0, blocking_to_running = 0, initial_to_running = 0;
	vector<int> originalAvailable = resourceVector;
	vector<int> availNextCycle(resourceNum+1, 0);
	
	//Process Each Instruction, start simulation
	while(true) {
		cycle += 1;
		
		// Try Initialization
		initial_to_running = 0;
		blocking_to_running = 0;
		banker_initialize(processVector, running, originalAvailable, processNum, resourceNum, initial_to_running, aborted, cycle);
		
		// add the resources available at this cycle to resourceVector
		for (int i = 1; i <= resourceNum; i++) {
			resourceVector[i] += availNextCycle[i];
			availNextCycle[i] = 0;
		}
		
		// before check the blocking list, tries to terminate some process first
	  // otherwise may have problem in checking "isSafeState" for blocking list
		tempRunning.clear();
		for (int i = 0; i < running.size(); i++) {
			// only care about the pattern <release(0delay) + terminate(0delay)>, to terminate
			bool cond1, cond2, cond3, cond4;
			cond1 = (running[i].getNextInstructionType() == "release");
			cond2 = (running[i].getNextDelay() == 0);
			cond3 = (running[i].getNextNextInstructionType() == "terminate");
			cond4 = (running[i].getNextNextDelay() == 0);
			if (cond1 && cond2 && cond3 && cond4) {
				release(running[i],availNextCycle);
				finished += 1;
				int index = findProcess(processVector, running[i].getRank());
				processVector[index].setFinish(cycle);
			}
			else {
				tempRunning.push_back(running[i]);
			}
		}
		running = tempRunning;
		tempRunning.clear();
		
		// check the blocking list, whose request might be satisfied now
		for (int i = 0; i < blocking.size(); i++) {
			int flag = isSafeState(processVector, running, blocking, processNum, resourceNum,
															blocking[i].getRank(), blocking[i].getNextInstruction(),
															resourceVector, availNextCycle, aborted, cycle);
			if (flag == 1) {
				Instruction instruction = blocking[i].getNextInstruction();
				int resourceType = instruction.getResourceType();
				int resourceQuantity = instruction.getResourceQuantity();
				blocking_to_running += 1;
				resourceVector[resourceType] -= resourceQuantity;
				blocking[i].addResource(resourceType, resourceQuantity);
				blocking[i].incrementIndex();
				running.push_back(blocking[i]);
			}
			else if (flag == 0) {
				tempBlocking.push_back(blocking[i]);
			}
		}
		blocking = tempBlocking;
		tempBlocking.clear();
		
	// processing the instructions in the running list
		for (int i = 0; i < running.size() - blocking_to_running - initial_to_running; i++) { // notice you don't touch those newly added process
			    // from initialization or blocking list
			    // because they are already "processed" in this cycle
			if (running[i].getNextInstructionType() == "request") {
				if (running[i].getNextDelay() == 0) { // immediate request
					int flag = isSafeState(processVector, running, blocking, processNum, resourceNum,
																 running[i].getRank(), running[i].getNextInstruction(),
																 resourceVector, availNextCycle, aborted, cycle);
					if (flag == 1) { // safe state
						Instruction instruction = running[i].getNextInstruction();
						int resourceType = instruction.getResourceType();
						int resourceQuantity = instruction.getResourceQuantity();
						resourceVector[resourceType] -= resourceQuantity;
						running[i].addResource(resourceType, resourceQuantity);
						running[i].incrementIndex();
						tempRunning.push_back(running[i]);
					}
					else if (flag == 0) { // unsafe state
						blocking.push_back(running[i]);
					}
				}
				else { // has delay, wait
					running[i].decreaseDelay();
					tempRunning.push_back(running[i]);
				}
			}
			else if (running[i].getNextInstructionType() == "release") {
				if (running[i].getNextDelay() == 0) { // immediate release
					release(running[i],availNextCycle);
					if (running[i].getNextInstructionType() == "terminate" && running[i].getNextDelay() == 0) { // try to do terminate as well
						int index = findProcess(processVector,running[i].getRank());
						processVector[index].setFinish(cycle);
						finished += 1;
					}
					else
						tempRunning.push_back(running[i]);
				}
				else { // has delay, wait
					running[i].decreaseDelay();
					tempRunning.push_back(running[i]);
				}
			}
			else if (running[i].getNextInstructionType() == "terminate") {
				int index = findProcess(processVector, running[i].getRank());
				if (running[i].getNextDelay() == 0) { // immediate terminate
					processVector[index].setFinish(cycle-1);
					finished += 1;
				}
				else { // has delay, wait
					running[i].decreaseDelay();
					tempRunning.push_back(running[i]);
				}
			}
		}
		for (int i = running.size()-blocking_to_running - initial_to_running; i < running.size(); i++)
		// we miss out those enter running list this cycle through initialization or blocking list, need to add them back
			tempRunning.push_back(running[i]);
		running = tempRunning;
		tempRunning.clear();
		blocking_to_running = 0;
		
		// add the wait time for process in the blocking list
		for (int i = 0; i < blocking.size(); i++) {
			int index = findProcess(processVector, blocking[i].getRank());
			processVector[index].addWait();
		}
		if (to_break)
			break;
		if (finished + aborted == processNum)
			to_break = true;
		if (cycle >= 500) {
			cout << "exit with cycle >= 500, I deem it as infinite loop (You can change that in the code if needed)" << endl;
			break;
		}
	}
	
	// print out statistics result to std::cout
	cout << "              BANKER'S" << endl;
	int total = 0, totalWait = 0;
	for (int i = 1; i <= processNum; i++) {
		cout << "     Task " << processVector[i].getRank() << "      ";
		if (processVector[i].isAbort())
			cout << "aborted" << endl;
		else {
			total += processVector[i].getFinish();
			totalWait += processVector[i].getWait();
			cout << processVector[i].getFinish() << "   " << processVector[i].getWait() << "  ";
			cout << int(0.5 + 100*(1.0*processVector[i].getWait())/processVector[i].getFinish()) << "%" << endl;
		}
	}
	cout << "     total       " << total << "   " << totalWait << "  ";
	if (total == 0)
		cout << "undefined" << endl;
	else {
		cout << int(0.5 + 100*(1.0*totalWait)/total) << "%" << endl;
	}
}

/*
 Function : optimistic
 the main function for running Optimistic Resource Allocator algorithm
 more details line by line inside
 we use the idea of filtering for list
*/
void optimistic(char* fileName,int detailed = 0) {
	// first do initialization of parameters
	ifstream ifs;
	openFile(fileName,ifs);
	int processNum, resourceNum, resourceCount;
	bool to_break = false;
	ifs >> processNum >> resourceNum;
	vector<Process> processVector(processNum+1);
	vector<int> resourceVector(resourceNum+1);
	vector<int> tempResourceVector;
	vector<Process> uninitiated, running, blocking, terminated;
	vector<Process> tempRunning, tempBlocking;
	for (int i = 1; i <= processNum; i++){
		processVector[i].setRank(i);
		processVector[i].setResourceTable(resourceNum);
	}
	for (int i = 1; i <= resourceNum; i++) {
		ifs >> resourceCount;
		resourceVector[i] = resourceCount;
	}
	while (ifs) {
		string instructionType, processRank, delay, resourceType, resourceQuantity;
		ifs >> instructionType >> processRank >> delay >> resourceType >> resourceQuantity;
		string givenInstruction = instructionType + " " + processRank + " " + delay + " " + resourceType + " " + resourceQuantity;
		int rank_toFind = atoi(processRank.c_str());
		int index = findProcess(processVector, rank_toFind);
		processVector[index].addInstruction(givenInstruction);
	}
	int cycle = 0, finished = 0, aborted = 0, blocking_to_running = 0, initial_to_running = 0;
	vector<int> availNextCycle(resourceNum+1, 0);
	
	// Processing Each Instruction, start simulation
	while (true) {
		cycle += 1;
		
		// Try Initialization
		initial_to_running = 0;
		initialize(processVector, running, uninitiated, resourceVector, processNum, resourceNum,initial_to_running, 0);
		
		// add the resources available at this cycle
		for (int i = 1; i <= resourceNum; i++) {
			resourceVector[i] += availNextCycle[i];
			availNextCycle[i] = 0;
		}
		
		// check the blocking list, whose request might be satisfied now
		for (int i = 0; i < blocking.size(); i++) {
			bool flag = request(blocking[i], resourceVector);
			if (flag == true) {
				blocking_to_running += 1;
				running.push_back(blocking[i]);
			}
			else {
				tempBlocking.push_back(blocking[i]);
			}
		}
		blocking = tempBlocking;
		tempBlocking.clear();
		
		// processing the Instructions in the running list
		for (int i = 0; i < running.size()-blocking_to_running-initial_to_running; i++) {
			// notice you don't touch those newly added process
			// from initialization or blocking list
			// because they are already "processed" in this cycle
			if (running[i].getNextInstructionType() == "request") { // immediate request
				if (running[i].getNextDelay() == 0) {
					bool flag = request(running[i],resourceVector);
					if (flag == true)
						tempRunning.push_back(running[i]);
					else
						blocking.push_back(running[i]);
				}
				else { // has delay, wait there
					running[i].decreaseDelay();
					tempRunning.push_back(running[i]);
					int index = findProcess(processVector, running[i].getRank());
				}
			}
			else if (running[i].getNextInstructionType() == "release") {
				if (running[i].getNextDelay() == 0) // immediate release
					release(running[i], availNextCycle);
				else { // has delay, wait there
					running[i].decreaseDelay();
					int index = findProcess(processVector, running[i].getRank());
				}
				tempRunning.push_back(running[i]);
			}
			else if (running[i].getNextInstructionType() == "terminate") {
				int index = findProcess(processVector, running[i].getRank());
				if (running[i].getNextDelay() == 0) { // immediate terminate
					processVector[index].setFinish(cycle-1);
					finished += 1;
				}
				else { //  has delay, wait there
					running[i].decreaseDelay();
					tempRunning.push_back(running[i]);
				}
		}
	}
		for ( int i = running.size()-blocking_to_running-initial_to_running; i < running.size(); i++)
			// we ignore those newly added process into running list through
			// initialization or initialization
			// so we need to add them back now
			tempRunning.push_back(running[i]);
		running = tempRunning;
		tempRunning.clear();
		blocking_to_running = 0;
		
		// check for possible deadlock
		// if happens, recursively abort the lowest index blocked process
		//    and release all its resource
		//    until no deadlock
		if (running.size() == 0 && blocking.size() > 0) {
			tempResourceVector = resourceVector;
			while (checkDeadlock(blocking, tempResourceVector)) {
				// abort and release the lowest index
				int lowestIndex = findLowestIndex(blocking);
				int blockingIndex = -1;
				for (Process process: blocking) {
					if (not (process.getRank() == lowestIndex)) {
						tempBlocking.push_back(process);
					}
				}
				for (int i = 0; i < blocking.size(); i++) {
					if (blocking[i].getRank() == lowestIndex)
						blockingIndex = i;
				}
				if (blockingIndex == -1) cout << "BlockingIndex == -1, dangerous!" << endl;
				int index = findProcess(processVector, lowestIndex);
				processVector[index].setAbort();
				releaseAbort(blocking[blockingIndex], availNextCycle, tempResourceVector);
				blocking = tempBlocking;
				tempBlocking.clear();
				aborted += 1;
			}
		}
		tempResourceVector.clear();
		
		// add the wait time for processes in blocking list
		for (int i = 0; i < blocking.size(); i++) {
			int index = findProcess(processVector, blocking[i].getRank());
			processVector[index].addWait();
		}
		if (to_break)
			break;
		if (finished + aborted == processNum)
			to_break = true;
		if (cycle >= 500) {
			cout << "exit with cycle >= 500, I deem it as infinite loop (You can change that in the code if needed)" << endl;
			break;
		}
	}
	// print out statistical result to std::cout
	cout << "              FIFO" << endl;
	int total = 0, totalWait = 0;
	for (int i = 1; i <= processNum; i++) {
		cout << "     Task " << processVector[i].getRank() << "      ";
		if (processVector[i].isAbort())
			cout << "aborted" << endl;
		else {
			total += processVector[i].getFinish();
			totalWait += processVector[i].getWait();
			cout << processVector[i].getFinish() << "   " << processVector[i].getWait() << "  ";
			cout << int(0.5 + 100*(1.0*processVector[i].getWait())/processVector[i].getFinish()) << "%" << endl;
		}
	}
	cout << "     total       " << total << "   " << totalWait << "  ";
	if (total == 0)
		cout << "undefined" << endl;
	else {
		cout << int(0.5 + 100*(1.0*totalWait)/total) << "%" << endl;
	}
}

/*
 Function : main
 main function drive
 start both Optimistic Resource Allocator and Banker's algorithm simulation
*/
int main(int argc, const char * argv[]) {
	char* fileName = (char*)argv[1];
	optimistic(fileName);
	cout << "--------------------------------------" << endl;
	banker(fileName);
	return 0;
}
