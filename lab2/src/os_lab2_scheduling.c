//
//  main.c
//  Lab 2
//
//  Created by Yukun,Jiang on 2/22/20.
//  Copyright © 2020 Yukun,Jiang. All rights reserved.
//
// In this file, I implemented 4 basic scheduling algorithms: FCFS,RR,LCFS and HPRN
// I first implement helper data structure: linked list with pointer * to Process struct
// Then we design 4 separate functions for these 4 scheduling algorithm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define UNSTARTED 0
#define RUNNING 1
#define BLOCKED 2
#define READY 3
#define FINISHED 4
#define QUANTUM 2


typedef struct Process{ /* my Process Class */
	unsigned int arrival; // A
	unsigned int randSize; // B
	unsigned int cpu;  // C
	unsigned int ioFactor; // M
	unsigned int ioRemain; // the remaining I/O time when blocked
	unsigned int originalBurst; // the original burst time, needed for --verbose output
	unsigned int ioTime; // the assigned ioTime when CPU burst time is over
	unsigned int waiting; // how long has this process waited in the system since entrance
	unsigned int totalIO; // the total I/O time this process did
	unsigned int id; // its id after sorting
	unsigned int oldID; // original id according to the order provided in the input file
	unsigned int finish; // when has this process finished
	unsigned int state; // 0,1,2,3,4 depending on its state
	unsigned int originalCPU; // the original CPU burst time assigned
	unsigned int preempted; // flag: if a process is preempted, set to 1. else 0
	unsigned int cpuRemain; // the remaining CPU time needed before this process terminates
	unsigned int T; // time in the system
	unsigned int cpuRun; // the real CPU running time had so far
	unsigned int t; // running time of process to data
	double r; // Penalty ratio
} Process;

typedef struct CPU{ /* my CPU Class */
	Process* master;
	int remainTime; // remaining CPU burst
	int finishTime; // the finish time assigned
	int waiting; // total empty waiting time for this CPU
	int cpuTime; // total CPU running time
	int ioTime; // total ioTime
	int quantum; // remaining quantum time used in RR algorithm
} CPU;
typedef struct node Node;

struct node { /* my Node Class used in Linked List */
	Process* data;
	struct node* next;
};

/* --------------------Linked List Functionality-----------------*/
void init(Node** head) {
	*head = NULL;
}

Node* getNext(Node* curr) {
	return curr->next;
}

int hasNext(Node* curr) {
	return curr != NULL;
}

Node* getLast(Node** head) {
	if (*head == NULL)
		return NULL;
	Node* curr = *head;
	while (curr->next != NULL)
		curr = curr->next;
	return curr;
}

void insertLast(Node** head, Process* process) {
	Node * newNode = (Node *)malloc(sizeof(Node));
	newNode->data = process;
	newNode->next = NULL;
	Node* lastNode = getLast(head);
	if (lastNode == NULL)
		*head = newNode;
	else
		lastNode->next = newNode;
}

Node* deleteFirst(Node** head) {
	if (*head == NULL){
		printf("Error: the head of Linked-list is empty.\n");
		return NULL;
	}
	else{
		Node* toReturn = *head;
		if (toReturn->next == NULL)
			*head = NULL;
		else
			*head = toReturn->next;
		return toReturn;
	}
}

int max(int number_A, int number_B){
	// helper function used in calculating Highest Penalty Ratio algorithm
	// t = max(t, 1)
	if (number_A <= number_B)
		return number_B;
	else
		return number_A;
}

int min3Number(int number_A, int number_B, int number_C){
	if ( number_A <= number_B && number_A <= number_C)
		return number_A;
	else if ( number_B <= number_A && number_B <= number_C)
		return number_B;
	else if (number_C <= number_A && number_C <= number_B)
		return number_C;
	else{
		printf("Something Wrong in the min function. Please check it again.\n");
		return -1;
	}
}

int min2Number( int number_A, int number_B){
	if (number_A < number_B)
		return number_A;
	return number_B;
}

int length(Node ** head){ // compute the length of a linked list
	if (*head == NULL)
		return 0;
	else{
		Node* curr =*head;
		int count = 0;
		while (curr != NULL){
			count += 1;
			curr = curr->next;
		}
		return count;
	}
}
void connect(Node** firstHead, Node** secondHead){
	// connect two linked list, link firstHead's tail to the head of secondHead
	if ( length(firstHead) == 0)
		*firstHead = *secondHead;
	else{
		Node* lastNode = getLast(firstHead);
		lastNode->next = *secondHead;
	}
}

void connectToHead(Node** firstHead, Node** secondHead){
	// put secondHead to top of firstHead, and change where firstHead pointed to
	if (length(secondHead) == 0)
		return;
	else{
		Node* temp = *firstHead;
		Node* Last = getLast(secondHead);
		*firstHead = *secondHead;
		Last->next = temp;
		return;
	}
}

void display(Node** head){
	if (length(head) > 0){
		Node* curr = *head;
		while (hasNext(curr)){
			printf("Process%d-->",curr->data->id);
			curr = curr->next;
		}
		printf("NULL\n");
	}
}

void clear(Node** head) {
	Node* curr = *head;
	while (hasNext(curr)) {
		Node* tmp = curr;
		curr = curr->next;
		free(tmp);
	}
	*head = NULL;
}
/*-----------------------End of Linked List Functionality--------------------*/

int randomOS(FILE* fpt, int randSize){ // use random seed to get the random number
	long long randomNum = -1;
	if (!fscanf(fpt,"%lld",&randomNum)){
		printf("Failure in reading in random number. Please check again.\n");
		return -1;
	}
	return (int)(1 + randomNum % randSize);
}

int myCompareByArrival(const void* processA, const void* processB){
	// Comparison function used in qsort, sort by process's arrivial time, then coming order
	Process* ptrA = *(Process**)processA;
	Process* ptrB = *(Process**)processB;
	if (ptrA->arrival < ptrB->arrival)
		return -1;
	else if (ptrA->arrival > ptrB->arrival)
		return 1;
	else
		return  (ptrA->oldID - ptrB->oldID);
}

int myCompareByArrivalStdout(const void* processA, const void* processB){
	// single pointer version of "myCompareByArrival" above
	Process* ptrA = (Process*)processA;
	Process* ptrB = (Process*)processB;
	if (ptrA->arrival < ptrB->arrival)
		return -1;
	else if (ptrA->arrival > ptrB->arrival)
		return 1;
	else
		return  (ptrA->oldID - ptrB->oldID);
}

int myCompareByHPR(const void* processA, const void* processB){
	// Comparison function used in qsort, sort by Penality ratio, then arrival, then coming order
	Process* ptrA = *(Process**)processA;
	Process* ptrB = *(Process**)processB;
	if (ptrA->r > ptrB->r)
		return -1;
	else if (ptrA->r < ptrB->r)
		return 1;
	else
		return myCompareByArrival((const void*)processA, (const void*)processB);
}

int myCompareByIO(const void* processA, const void* processB){
	// Comparison function used in qsort, sort by I/O remaining time, then arrival, then coming order
	Process* ptrA = *(Process**)processA;
	Process* ptrB = *(Process**)processB;
	if (ptrA->ioTime < ptrB->ioTime)
		return -1;
	else if (ptrA->ioTime > ptrB->ioTime)
		return 1;
	else
		return myCompareByArrival((const void*)processA, (const void*)processB);
}

void sort(Node** head, int mode){
	// sort a linked list by a particular criterion
	// First store in an array, sort the array, then re-construct the linked list
	int len = length(head);
	if (len == 0 || len == 1)
		return;
	else{
		Process* processList[len];
		int walk=0;
		Node* curr = *head;
		while (curr != NULL){
			processList[walk] = curr->data;
			walk += 1;
			curr = curr->next;
	}
		if (mode == 0)
			qsort(processList,len,sizeof(Node *),myCompareByArrival);
		else if (mode == 1)
			qsort(processList,len,sizeof(Node *),myCompareByIO);
		else if (mode == 2)
			qsort(processList,len,sizeof(Node *),myCompareByHPR);
		Node * newhead;
		init(&newhead);
		for (int i = 0; i < len; i++){
			insertLast(&newhead, processList[i]);
		}
		clear(head);
		*head = newhead;
	}
}

void print_summary(Process processList[], int processNum, CPU myCPU){
	// helper function, called at the end of each algorithm to print out summary information
	int sumTurnaround = 0;
	int sumWaiting = 0;
	for (int i = 0; i < processNum; i++){
		sumTurnaround += (processList[i].finish - processList[i].arrival);
		sumWaiting += processList[i].waiting;
		printf("Process %d:\n",i);
		printf("\t(A,B,C,M) = (%d,%d,%d,%d)\n",processList[i].arrival,processList[i].randSize,processList[i].originalCPU,processList[i].ioFactor);
		printf("\tFinishing time: %d\n",processList[i].finish);
		printf("\tTurnaround time: %d\n",processList[i].finish - processList[i].arrival);
		printf("\tI/O time: %d\n",processList[i].totalIO);
		printf("\tWaiting time: %d\n",processList[i].waiting);
		printf("\n");
	}
	printf("Summary Data:\n");
	printf("\tFinishing time: %d\n",myCPU.finishTime);
	printf("\tCPU Utilization: %0.6f\n",(1.0*myCPU.cpuTime)/myCPU.finishTime);
	printf("\tI/O Utilization: %0.6f\n",(1.0*myCPU.ioTime)/myCPU.finishTime);
	printf("\tThroughput: %0.6f processes per hundred cycles\n",(100.0*processNum)/myCPU.finishTime);
	printf("\tAverage turnaround time: %0.6f\n",(1.0*sumTurnaround)/processNum);
	printf("\tAverage waiting time: %0.6f\n",(1.0*sumWaiting)/processNum);
}

void FCFS(Process processList[], int processNum, int verbose) {
	FILE * fpt = fopen("random-numbers.txt","r");
	CPU myCPU;
	int clockTime = 0;
	int finishedProcess = 0;
	myCPU.master = NULL;
	myCPU.remainTime = 0;
	myCPU.waiting = 0;
	myCPU.finishTime = 0;
	myCPU.ioTime = 0;
	myCPU.cpuTime = 0;
	myCPU.quantum = 0;
	Node* readyHead;
	Node* ioHead;
	Node* waitHead;
	Node* tempHead;
	init((Node**)&readyHead);
	init((Node**)&ioHead);
	init((Node**)&waitHead);
	init((Node**)&tempHead);

	// std:out
	printf("The original input was: %d ",processNum);
	for ( int i = 0; i < processNum; i++){
		printf("(%d %d %d %d) ",processList[i].arrival,processList[i].randSize,processList[i].originalCPU,processList[i].ioFactor);
	}
	printf("\n");
	qsort(processList,processNum,sizeof(Process),myCompareByArrivalStdout);
	printf("The (sorted) input is:  %d ",processNum);
	for ( int i = 0; i < processNum; i++){
		printf("(%d %d %d %d) ",processList[i].arrival,processList[i].randSize,processList[i].originalCPU,processList[i].ioFactor);
	}
	printf("\n\n");
	if (verbose == 1)
		printf("This detailed printout gives the state and remaining burst for each process\n\n");
	
	// reorder the process's id, since we have sorted them once
	for ( int i = 0; i < processNum; i++ )
		processList[i].id = i;
	
	// initialize the list before first CPU cycle run
	/*for (int i = 0; i < processNum; i++){
		if (processList[i].arrival == 0){
			processList[i].state = READY;
			insertLast((Node**)&readyHead, &processList[i]);
		}
		else{
			processList[i].state = UNSTARTED;
			insertLast((Node**)&waitHead, &processList[i]);
		}
	} */
	// std:out verbose hardcoding for cycle -1
	if (verbose == 1){
		printf("Before cycle%5d: ",0);
		for ( int i = 0; i < processNum; i++){
			if (processList[i].state == UNSTARTED)
				printf("  unstarted  0");
			else if (processList[i].state == RUNNING)
				printf("    running%3d",processList[i].originalBurst);
			else if (processList[i].state == READY)
				printf("      ready  0");
			else if (processList[i].state == BLOCKED)
				printf("    blocked%3d",processList[i].ioTime);
			else if (processList[i].state == FINISHED)
				printf(" terminated  0");
		}
		printf(".\n");
	}
		
	while(1)
	{
		if (clockTime == 0) { // initialize two lists: waitHead & readyHead
			for (int i = 0; i < processNum; i++){
				if (processList[i].arrival == 0){
					processList[i].state = READY;
					insertLast((Node**)&readyHead, &processList[i]);
				}
				else{
					processList[i].state = UNSTARTED;
					insertLast((Node**)&waitHead, &processList[i]);
				}
			}
		}
		// search for arrival process, add them to ready list
		sort(&waitHead,0);
		while (1) {
			Node* curr = waitHead;
			if (curr == NULL || curr->data->arrival != clockTime) /* use short-circuit evaluation */
				break;
			else{
				Node* readyNode = deleteFirst(&waitHead);
				readyNode->data->state = READY;
				insertLast(&tempHead, readyNode->data);
			}
		}
		
		// search for finished I/O process, add them to ready list
		sort(&ioHead,1);
		while (1) {
			Node* curr = ioHead;
			if (curr == NULL || curr->data->ioTime > 0)
				break;
			else{
				Node* finishioNode = deleteFirst(&ioHead);
				finishioNode->data->state = READY;
				insertLast(&tempHead, finishioNode->data);
			}
		}
		
		// concatenate the new ready process to the ready list
		sort(&tempHead,0);
		connect(&readyHead, &tempHead);
		init(&tempHead);
		
		// check CPU usage, potentially switch the master on the CPU
		if (myCPU.master == NULL){ /* nobody is using the CPU, try to find one from the ready list */
			if (length(&readyHead) > 0){
				Node* toCPUNode = deleteFirst(&readyHead);
				toCPUNode->data->state = RUNNING;
				myCPU.master = toCPUNode->data;
				int randomNumber = randomOS(fpt, myCPU.master->randSize);
				myCPU.master->originalBurst = randomNumber;
				myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
				myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
			}
			else{ // length of ready list = 0, waiting
				;
			}
		}
		else { /* CPU has a master right now. Check if it out of CPU burst time */
			if (myCPU.master->cpu == 0){ /* the current master has finished*/
				myCPU.master->state = FINISHED;
				myCPU.master->finish = clockTime;
				finishedProcess += 1;
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				if (length(&readyHead) > 0){
					Node* toCPUNode = deleteFirst(&readyHead);
					toCPUNode->data->state = RUNNING;
					myCPU.master = toCPUNode->data;
					int randomNumber = randomOS(fpt, myCPU.master->randSize);
					myCPU.master->originalBurst = randomNumber;
					myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
					myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
				}
			}
			else if (myCPU.remainTime == 0){ /* current CPU burst out finishes, switch process */
				myCPU.master->state = BLOCKED;
				insertLast(&ioHead,myCPU.master);
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				if (length(&readyHead) > 0){
					Node* toCPUNode = deleteFirst(&readyHead);
					toCPUNode->data->state = RUNNING;
					myCPU.master = toCPUNode->data;
					int randomNumber = randomOS(fpt, myCPU.master->randSize);
					myCPU.master->originalBurst = randomNumber;
					myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
					myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
				}
			}
		}
		// std:out verbose
		if (verbose == 1 && finishedProcess != processNum){
			printf("Before cycle%5d: ",clockTime+1);
			for ( int i = 0; i < processNum; i++){
				if (processList[i].state == UNSTARTED)
					printf("  unstarted  0");
				else if (processList[i].state == RUNNING)
					printf("    running%3d",processList[i].originalBurst);
				else if (processList[i].state == READY)
					printf("      ready  0");
				else if (processList[i].state == BLOCKED)
					printf("    blocked%3d",processList[i].ioTime);
				else if (processList[i].state == FINISHED)
					printf(" terminated  0");
				}
			printf(".\n");
		}
		// run this cycle-----1. run the CPU 2. run the I/O 3. increment the waiting time
		/* run the CPU */
		if (myCPU.master != NULL){
			myCPU.remainTime -= 1;
			myCPU.master->cpu -= 1;
			myCPU.master->originalBurst -= 1;
		}
		/* run the I/O */
		Node* walker = ioHead;
		for (int i = 0; i < length(&ioHead); i++){
			walker->data->ioTime -= 1;
			walker->data->totalIO += 1;
			walker = walker->next;
		}
		/* increment the waiting time */
		walker = readyHead;
		for( int i = 0; i < length(&readyHead); i++){
			walker->data->waiting += 1;
			walker = walker->next;
		}
		
		// increase the clockTime, check if all processes have finished
		clockTime += 1;

		if (clockTime > 100000 || finishedProcess == processNum){
			if (length(&ioHead) != 0)
				printf("ioHead(%d) is not empty yet.\n",length(&ioHead));
			if (length(&readyHead) != 0)
				printf("readyHead(%d) is not empty yet.\n",length(&readyHead));
			if (length(&waitHead) != 0)
				printf("waitHead(%d) is not empty yet.\n",length(&waitHead));
			break;
		}
		if (length(&ioHead) > 0)
			myCPU.ioTime += 1;
		if (myCPU.master != NULL)
			myCPU.cpuTime += 1;
	}
	
	myCPU.finishTime = clockTime - 1;
	printf("The scheduling algorithm used was First Come First Served\n\n");
	print_summary(processList,processNum,myCPU);
	fclose(fpt);
}

void RR(Process processList[], int processNum, int verbose) {
	FILE * fpt = fopen("random-numbers.txt","r");
	CPU myCPU;
	int clockTime = 0;
	int finishedProcess = 0;
	myCPU.master = NULL;
	myCPU.remainTime = 0;
	myCPU.waiting = 0;
	myCPU.finishTime = 0;
	myCPU.ioTime = 0;
	myCPU.cpuTime = 0;
	myCPU.quantum = 0;
	Node* readyHead;
	Node* ioHead;
	Node* waitHead;
	Node* tempHead;
	init((Node**)&readyHead);
	init((Node**)&ioHead);
	init((Node**)&waitHead);
	init((Node**)&tempHead);

	// std:out
	printf("The original input was: %d ",processNum);
	for ( int i = 0; i < processNum; i++){
		printf("(%d %d %d %d) ",processList[i].arrival,processList[i].randSize,processList[i].originalCPU,processList[i].ioFactor);
	}
	printf("\n");
	qsort(processList,processNum,sizeof(Process),myCompareByArrivalStdout);
	printf("The (sorted) input is:  %d ",processNum);
	for ( int i = 0; i < processNum; i++){
		printf("(%d %d %d %d) ",processList[i].arrival,processList[i].randSize,processList[i].originalCPU,processList[i].ioFactor);
	}
	printf("\n\n");
	if (verbose == 1)
		printf("This detailed printout gives the state and remaining burst for each process\n\n");
	
	// reorder the process's id, since we have sorted them once
	for ( int i = 0; i < processNum; i++ )
		processList[i].id = i;
	
	// initialize the list before first CPU cycle run
	/*for (int i = 0; i < processNum; i++){
		if (processList[i].arrival == 0){
			processList[i].state = READY;
			insertLast((Node**)&readyHead, &processList[i]);
		}
		else{
			processList[i].state = UNSTARTED;
			insertLast((Node**)&waitHead, &processList[i]);
		}
	} */
	// std:out verbose hardcoding for cycle -1
	if (verbose == 1){
		printf("Before cycle%5d: ",0);
		for ( int i = 0; i < processNum; i++){
			if (processList[i].state == UNSTARTED)
				printf("  unstarted  0");
			else if (processList[i].state == RUNNING)
				printf("    running%3d",processList[i].originalBurst);
			else if (processList[i].state == READY)
				printf("      ready  0");
			else if (processList[i].state == BLOCKED)
				printf("    blocked%3d",processList[i].ioTime);
			else if (processList[i].state == FINISHED)
				printf(" terminated  0");
		}
		printf(".\n");
	}
		
	while(1)
	{
		if (clockTime == 0) { // initialize two lists: waitHead & readyHead
			for (int i = 0; i < processNum; i++){
				if (processList[i].arrival == 0){
					processList[i].state = READY;
					insertLast((Node**)&readyHead, &processList[i]);
				}
				else{
					processList[i].state = UNSTARTED;
					insertLast((Node**)&waitHead, &processList[i]);
				}
			}
		}
		// search for arrival process, add them to ready list
		sort(&waitHead,0);
		while (1) {
			Node* curr = waitHead;
			if (curr == NULL || curr->data->arrival != clockTime) /* use short-circuit evaluation */
				break;
			else{
				Node* readyNode = deleteFirst(&waitHead);
				readyNode->data->state = READY;
				insertLast(&tempHead, readyNode->data);
			}
		}
		
		// search for finished I/O process, add them to ready list
		sort(&ioHead,1);
		while (1) {
			Node* curr = ioHead;
			if (curr == NULL || curr->data->ioTime > 0)
				break;
			else{
				Node* finishioNode = deleteFirst(&ioHead);
				finishioNode->data->state = READY;
				insertLast(&tempHead, finishioNode->data);
			}
		}
		
		if (myCPU.master != NULL)
		{
			if (myCPU.master->cpu != 0 && myCPU.remainTime != 0 && myCPU.quantum == 0){
				myCPU.master->state = READY;
				myCPU.master->preempted = 1;
				myCPU.master->cpuRemain = myCPU.remainTime;
				insertLast(&tempHead, myCPU.master);
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				myCPU.quantum = 0;
				}
			}
	
		// concatenate the new ready process to the ready list
		sort(&tempHead,0);
		connect(&readyHead, &tempHead);
		init(&tempHead);
		
		// check CPU usage, potentially switch the master on the CPU
		if (myCPU.master == NULL){ /* nobody is using the CPU, try to find one from the ready list */
			if (length(&readyHead) > 0){
				Node* toCPUNode = deleteFirst(&readyHead);
				toCPUNode->data->state = RUNNING;
				myCPU.master = toCPUNode->data;
				myCPU.quantum = QUANTUM;
				if (myCPU.master->preempted == 0){
				int randomNumber = randomOS(fpt, myCPU.master->randSize);
				myCPU.master->originalBurst = randomNumber;
				myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
				myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
				}
				else if (myCPU.master->preempted == 1){
					myCPU.master->preempted = 0;
					myCPU.remainTime = myCPU.master->cpuRemain;
					myCPU.master->cpuRemain = 0;
				}
			}
			else{ // length of ready list = 0, waiting
				;
			}
		}
		else { /* CPU has a master right now. Check if it out of CPU burst time */
			if (myCPU.master->cpu == 0){ /* the current master has finished*/
				myCPU.master->state = FINISHED;
				myCPU.master->finish = clockTime;
				finishedProcess += 1;
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				myCPU.quantum = 0;
				if (length(&readyHead) > 0){
					Node* toCPUNode = deleteFirst(&readyHead);
					toCPUNode->data->state = RUNNING;
					myCPU.master = toCPUNode->data;
					myCPU.quantum = QUANTUM;
					if (myCPU.master->preempted == 0){
					int randomNumber = randomOS(fpt, myCPU.master->randSize);
					myCPU.master->originalBurst = randomNumber;
					myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
					myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
					}
					else if (myCPU.master->preempted == 1){
						myCPU.master->preempted = 0;
						myCPU.remainTime = myCPU.master->cpuRemain;
						myCPU.master->cpuRemain = 0;
					}
				}
			}
			else if (myCPU.remainTime == 0){ /* current CPU burst out finishes, switch process to I/O */
				myCPU.master->state = BLOCKED;
				insertLast(&ioHead,myCPU.master);
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				myCPU.quantum = 0;
				if (length(&readyHead) > 0){
					Node* toCPUNode = deleteFirst(&readyHead);
					toCPUNode->data->state = RUNNING;
					myCPU.master = toCPUNode->data;
					myCPU.quantum = QUANTUM;
					if (myCPU.master->preempted == 0){
					int randomNumber = randomOS(fpt, myCPU.master->randSize);
					myCPU.master->originalBurst = randomNumber;
					myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
					myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
					}
					else if (myCPU.master->preempted == 1){
						myCPU.master->preempted = 0;
						myCPU.remainTime = myCPU.master->cpuRemain;
						myCPU.master->cpuRemain = 0;
					}
				}
			}
			else if (myCPU.quantum == 0){
				/* this quantum is over. Add running process to the back of ready list */
				/* need to save the state: i.e. remaining CPU burst before doing I/O*/
				/* don't initialize new randomOS() number */
				myCPU.master->state = READY;
				myCPU.master->preempted = 1;
				myCPU.master->cpuRemain = myCPU.remainTime;
				insertLast(&readyHead, myCPU.master);
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				myCPU.quantum = 0;
				if (length(&readyHead) > 0){
					Node* toCPUNode = deleteFirst(&readyHead);
					toCPUNode->data->state = RUNNING;
					myCPU.master = toCPUNode->data;
					myCPU.quantum = QUANTUM;
					if (myCPU.master->preempted == 0){
						int randomNumber = randomOS(fpt, myCPU.master->randSize);
						myCPU.master->originalBurst = randomNumber;
						myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
						myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
					}
					else if (myCPU.master->preempted == 1){
						myCPU.master->preempted = 0;
						myCPU.remainTime = myCPU.master->cpuRemain;
						myCPU.master->cpuRemain = 0;
					}
				}
			}
		}
		// std:out verbose
		if (verbose == 1 && finishedProcess != processNum){
			printf("Before cycle%5d: ",clockTime+1);
			for ( int i = 0; i < processNum; i++){
				if (processList[i].state == UNSTARTED)
					printf("  unstarted  0");
				else if (processList[i].state == RUNNING)
					printf("    running%3d",min2Number(myCPU.quantum,processList[i].originalBurst));
				else if (processList[i].state == READY)
					printf("      ready  0");
				else if (processList[i].state == BLOCKED)
					printf("    blocked%3d",processList[i].ioTime);
				else if (processList[i].state == FINISHED)
					printf(" terminated  0");
				}
			printf(".\n");
		}
		// run this cycle-----1. run the CPU, decrease quantum 2. run the I/O 3. increment the waiting time
		/* run the CPU */
		if (myCPU.master != NULL){
			myCPU.remainTime -= 1;
			myCPU.master->cpu -= 1;
			myCPU.master->originalBurst -= 1;
			myCPU.quantum -= 1;
		}
		/* run the I/O */
		Node* walker = ioHead;
		for (int i = 0; i < length(&ioHead); i++){
			walker->data->ioTime -= 1;
			walker->data->totalIO += 1;
			walker = walker->next;
		}
		/* increment the waiting time */
		walker = readyHead;
		for( int i = 0; i < length(&readyHead); i++){
			walker->data->waiting += 1;
			walker = walker->next;
		}
		
		// increase the clockTime, check if all processes have finished
		clockTime += 1;

		if (clockTime > 100000 || finishedProcess == processNum){
			if (length(&ioHead) != 0)
				printf("ioHead(%d) is not empty yet.\n",length(&ioHead));
			if (length(&readyHead) != 0)
				printf("readyHead(%d) is not empty yet.\n",length(&readyHead));
			if (length(&waitHead) != 0)
				printf("waitHead(%d) is not empty yet.\n",length(&waitHead));
			break;
		}
		if (length(&ioHead) > 0)
			myCPU.ioTime += 1;
		if (myCPU.master != NULL)
			myCPU.cpuTime += 1;
	}
	
	myCPU.finishTime = clockTime - 1;
	printf("The scheduling algorithm used was Round Robbin\n\n");
	print_summary(processList,processNum,myCPU);
	fclose(fpt);
}

void LCFS(Process processList[], int processNum, int verbose) {
	FILE * fpt = fopen("random-numbers.txt","r");
	CPU myCPU;
	int clockTime = 0;
	int finishedProcess = 0;
	myCPU.master = NULL;
	myCPU.remainTime = 0;
	myCPU.waiting = 0;
	myCPU.finishTime = 0;
	myCPU.ioTime = 0;
	myCPU.cpuTime = 0;
	myCPU.quantum = 0;
	Node* readyHead;
	Node* ioHead;
	Node* waitHead;
	Node* tempHead;
	init((Node**)&readyHead);
	init((Node**)&ioHead);
	init((Node**)&waitHead);
	init((Node**)&tempHead);

	// std:out
	printf("The original input was: %d ",processNum);
	for ( int i = 0; i < processNum; i++){
		printf("(%d %d %d %d) ",processList[i].arrival,processList[i].randSize,processList[i].originalCPU,processList[i].ioFactor);
	}
	printf("\n");
	qsort(processList,processNum,sizeof(Process),myCompareByArrivalStdout);
	printf("The (sorted) input is:  %d ",processNum);
	for ( int i = 0; i < processNum; i++){
		printf("(%d %d %d %d) ",processList[i].arrival,processList[i].randSize,processList[i].originalCPU,processList[i].ioFactor);
	}
	printf("\n\n");
	if (verbose == 1)
		printf("This detailed printout gives the state and remaining burst for each process\n\n");
	
	// reorder the process's id, since we have sorted them once
	for ( int i = 0; i < processNum; i++ )
		processList[i].id = i;
	
	// initialize the list before first CPU cycle run
	/*for (int i = 0; i < processNum; i++){
		if (processList[i].arrival == 0){
			processList[i].state = READY;
			insertLast((Node**)&readyHead, &processList[i]);
		}
		else{
			processList[i].state = UNSTARTED;
			insertLast((Node**)&waitHead, &processList[i]);
		}
	} */
	// std:out verbose hardcoding for cycle -1
	if (verbose == 1){
		printf("Before cycle%5d: ",0);
		for ( int i = 0; i < processNum; i++){
			if (processList[i].state == UNSTARTED)
				printf("  unstarted  0");
			else if (processList[i].state == RUNNING)
				printf("    running%3d",processList[i].originalBurst);
			else if (processList[i].state == READY)
				printf("      ready  0");
			else if (processList[i].state == BLOCKED)
				printf("    blocked%3d",processList[i].ioTime);
			else if (processList[i].state == FINISHED)
				printf(" terminated  0");
		}
		printf(".\n");
	}
		
	while(1)
	{
		if (clockTime == 0) { // initialize two lists: waitHead & readyHead
			for (int i = 0; i < processNum; i++){
				if (processList[i].arrival == 0){
					processList[i].state = READY;
					insertLast((Node**)&readyHead, &processList[i]);
				}
				else{
					processList[i].state = UNSTARTED;
					insertLast((Node**)&waitHead, &processList[i]);
				}
			}
		}
		// search for arrival process, add them to ready list( The head of ready list because of LCFS)
		sort(&waitHead,0);
		while (1) {
			Node* curr = waitHead;
			if (curr == NULL || curr->data->arrival != clockTime) /* use short-circuit evaluation */
				break;
			else{
				Node* readyNode = deleteFirst(&waitHead);
				readyNode->data->state = READY;
				insertLast(&tempHead, readyNode->data);
			}
		}
		
		// search for finished I/O process, add them to ready list
		sort(&ioHead,1);
		while (1) {
			Node* curr = ioHead;
			if (curr == NULL || curr->data->ioTime > 0)
				break;
			else{
				Node* finishioNode = deleteFirst(&ioHead);
				finishioNode->data->state = READY;
				insertLast(&tempHead, finishioNode->data);
			}
		}
		
		// concatenate the new ready process to the ready list
		sort(&tempHead,0);
		connectToHead(&readyHead,&tempHead);
		init(&tempHead);
		
		// check CPU usage, potentially switch the master on the CPU
		if (myCPU.master == NULL){ /* nobody is using the CPU, try to find one from the ready list */
			if (length(&readyHead) > 0){
				Node* toCPUNode = deleteFirst(&readyHead);
				toCPUNode->data->state = RUNNING;
				myCPU.master = toCPUNode->data;
				int randomNumber = randomOS(fpt, myCPU.master->randSize);
				myCPU.master->originalBurst = randomNumber;
				myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
				myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
			}
			else{ // length of ready list = 0, waiting
				;
			}
		}
		else { /* CPU has a master right now. Check if it out of CPU burst time */
			if (myCPU.master->cpu == 0){ /* the current master has finished*/
				myCPU.master->state = FINISHED;
				myCPU.master->finish = clockTime;
				finishedProcess += 1;
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				if (length(&readyHead) > 0){
					Node* toCPUNode = deleteFirst(&readyHead);
					toCPUNode->data->state = RUNNING;
					myCPU.master = toCPUNode->data;
					int randomNumber = randomOS(fpt, myCPU.master->randSize);
					myCPU.master->originalBurst = randomNumber;
					myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
					myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
				}
			}
			else if (myCPU.remainTime == 0){ /* current CPU burst out finishes, switch process */
				myCPU.master->state = BLOCKED;
				insertLast(&ioHead,myCPU.master);
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				if (length(&readyHead) > 0){
					Node* toCPUNode = deleteFirst(&readyHead);
					toCPUNode->data->state = RUNNING;
					myCPU.master = toCPUNode->data;
					int randomNumber = randomOS(fpt, myCPU.master->randSize);
					myCPU.master->originalBurst = randomNumber;
					myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
					myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
				}
			}
		}
		// std:out verbose
		if (verbose == 1 && finishedProcess != processNum){
			printf("Before cycle%5d: ",clockTime+1);
			for ( int i = 0; i < processNum; i++){
				if (processList[i].state == UNSTARTED)
					printf("  unstarted  0");
				else if (processList[i].state == RUNNING)
					printf("    running%3d",processList[i].originalBurst);
				else if (processList[i].state == READY)
					printf("      ready  0");
				else if (processList[i].state == BLOCKED)
					printf("    blocked%3d",processList[i].ioTime);
				else if (processList[i].state == FINISHED)
					printf(" terminated  0");
				}
			printf(".\n");
		}
		// run this cycle-----1. run the CPU 2. run the I/O 3. increment the waiting time
		/* run the CPU */
		if (myCPU.master != NULL){
			myCPU.remainTime -= 1;
			myCPU.master->cpu -= 1;
			myCPU.master->originalBurst -= 1;
		}
		/* run the I/O */
		Node* walker = ioHead;
		for (int i = 0; i < length(&ioHead); i++){
			walker->data->ioTime -= 1;
			walker->data->totalIO += 1;
			walker = walker->next;
		}
		/* increment the waiting time */
		walker = readyHead;
		for( int i = 0; i < length(&readyHead); i++){
			walker->data->waiting += 1;
			walker = walker->next;
		}
		
		// increase the clockTime, check if all processes have finished
		clockTime += 1;

		if (clockTime > 100000 || finishedProcess == processNum){
			if (length(&ioHead) != 0)
				printf("ioHead(%d) is not empty yet.\n",length(&ioHead));
			if (length(&readyHead) != 0)
				printf("readyHead(%d) is not empty yet.\n",length(&readyHead));
			if (length(&waitHead) != 0)
				printf("waitHead(%d) is not empty yet.\n",length(&waitHead));
			break;
		}
		if (length(&ioHead) > 0)
			myCPU.ioTime += 1;
		if (myCPU.master != NULL)
			myCPU.cpuTime += 1;
	}
	
	myCPU.finishTime = clockTime - 1;
	printf("The scheduling algorithm used was Last Come First Served\n\n");
	print_summary(processList,processNum,myCPU);
	fclose(fpt);
}

void HPRN(Process processList[], int processNum, int verbose) {
	FILE * fpt = fopen("random-numbers.txt","r");
	CPU myCPU;
	int clockTime = 0;
	int finishedProcess = 0;
	myCPU.master = NULL;
	myCPU.remainTime = 0;
	myCPU.waiting = 0;
	myCPU.finishTime = 0;
	myCPU.ioTime = 0;
	myCPU.cpuTime = 0;
	myCPU.quantum = 0;
	Node* readyHead;
	Node* ioHead;
	Node* waitHead;
	Node* tempHead;
	init((Node**)&readyHead);
	init((Node**)&ioHead);
	init((Node**)&waitHead);
	init((Node**)&tempHead);

	// std:out
	printf("The original input was: %d ",processNum);
	for ( int i = 0; i < processNum; i++){
		printf("(%d %d %d %d) ",processList[i].arrival,processList[i].randSize,processList[i].originalCPU,processList[i].ioFactor);
	}
	printf("\n");
	qsort(processList,processNum,sizeof(Process),myCompareByArrivalStdout);
	printf("The (sorted) input is:  %d ",processNum);
	for ( int i = 0; i < processNum; i++){
		printf("(%d %d %d %d) ",processList[i].arrival,processList[i].randSize,processList[i].originalCPU,processList[i].ioFactor);
	}
	printf("\n\n");
	if (verbose == 1)
		printf("This detailed printout gives the state and remaining burst for each process\n\n");
	
	// reorder the process's id, since we have sorted them once
	for ( int i = 0; i < processNum; i++ )
		processList[i].id = i;
	
	// initialize the list before first CPU cycle run
	/*for (int i = 0; i < processNum; i++){
		if (processList[i].arrival == 0){
			processList[i].state = READY;
			insertLast((Node**)&readyHead, &processList[i]);
		}
		else{
			processList[i].state = UNSTARTED;
			insertLast((Node**)&waitHead, &processList[i]);
		}
	} */
	// std:out verbose hardcoding for cycle -1
	if (verbose == 1){
		printf("Before cycle%5d: ",0);
		for ( int i = 0; i < processNum; i++){
			if (processList[i].state == UNSTARTED)
				printf("  unstarted  0");
			else if (processList[i].state == RUNNING)
				printf("    running%3d",processList[i].originalBurst);
			else if (processList[i].state == READY)
				printf("      ready  0");
			else if (processList[i].state == BLOCKED)
				printf("    blocked%3d",processList[i].ioTime);
			else if (processList[i].state == FINISHED)
				printf(" terminated  0");
		}
		printf(".\n");
	}
		
	while(1)
	{
		if (clockTime == 0) { // initialize two lists: waitHead & readyHead
			for (int i = 0; i < processNum; i++){
				if (processList[i].arrival == 0){
					processList[i].state = READY;
					insertLast((Node**)&readyHead, &processList[i]);
				}
				else{
					processList[i].state = UNSTARTED;
					insertLast((Node**)&waitHead, &processList[i]);
				}
			}
		}
		// search for arrival process, add them to ready list
		sort(&waitHead,0);
		while (1) {
			Node* curr = waitHead;
			if (curr == NULL || curr->data->arrival != clockTime) /* use short-circuit evaluation */
				break;
			else{
				Node* readyNode = deleteFirst(&waitHead);
				readyNode->data->state = READY;
				insertLast(&tempHead, readyNode->data);
			}
		}
		
		// search for finished I/O process, add them to ready list
		sort(&ioHead,1);
		while (1) {
			Node* curr = ioHead;
			if (curr == NULL || curr->data->ioTime > 0)
				break;
			else{
				Node* finishioNode = deleteFirst(&ioHead);
				finishioNode->data->state = READY;
				insertLast(&tempHead, finishioNode->data);
			}
		}
		
		// concatenate the new ready process to the ready list
		sort(&tempHead,0);
		connect(&readyHead, &tempHead);
		// calculate the penalty ratio for each process in the ready list
		Node* curr = readyHead;
		while (curr != NULL){
			curr->data->T = clockTime - curr->data->arrival;
			curr->data->t = max(curr->data->cpuRun, 1);
			curr->data->r = (1.0*curr->data->T) / (curr->data->t);
			curr = curr->next;
		}
		sort(&readyHead,2); /* sort the ready list by corresponding Penalty ratio */
		init(&tempHead);
		
		// check CPU usage, potentially switch the master on the CPU
		if (myCPU.master == NULL){ /* nobody is using the CPU, try to find one from the ready list */
			if (length(&readyHead) > 0){
				Node* toCPUNode = deleteFirst(&readyHead);
				toCPUNode->data->state = RUNNING;
				myCPU.master = toCPUNode->data;
				int randomNumber = randomOS(fpt, myCPU.master->randSize);
				myCPU.master->originalBurst = randomNumber;
				myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
				myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
			}
			else{ // length of ready list = 0, waiting
				;
			}
		}
		else { /* CPU has a master right now. Check if it out of CPU burst time */
			if (myCPU.master->cpu == 0){ /* the current master has finished*/
				myCPU.master->state = FINISHED;
				myCPU.master->finish = clockTime;
				finishedProcess += 1;
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				if (length(&readyHead) > 0){
					Node* toCPUNode = deleteFirst(&readyHead);
					toCPUNode->data->state = RUNNING;
					myCPU.master = toCPUNode->data;
					int randomNumber = randomOS(fpt, myCPU.master->randSize);
					myCPU.master->originalBurst = randomNumber;
					myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
					myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
				}
			}
			else if (myCPU.remainTime == 0){ /* current CPU burst out finishes, switch process */
				myCPU.master->state = BLOCKED;
				insertLast(&ioHead,myCPU.master);
				myCPU.master = NULL;
				myCPU.remainTime = 0;
				if (length(&readyHead) > 0){
					Node* toCPUNode = deleteFirst(&readyHead);
					toCPUNode->data->state = RUNNING;
					myCPU.master = toCPUNode->data;
					int randomNumber = randomOS(fpt, myCPU.master->randSize);
					myCPU.master->originalBurst = randomNumber;
					myCPU.remainTime = (myCPU.master->cpu > randomNumber) ? randomNumber : myCPU.master->cpu;
					myCPU.master->ioTime = myCPU.remainTime * myCPU.master->ioFactor;
				}
			}
		}
		// std:out verbose
		if (verbose == 1 && finishedProcess != processNum){
			printf("Before cycle%5d: ",clockTime+1);
			for ( int i = 0; i < processNum; i++){
				if (processList[i].state == UNSTARTED)
					printf("  unstarted  0");
				else if (processList[i].state == RUNNING)
					printf("    running%3d",processList[i].originalBurst);
				else if (processList[i].state == READY)
					printf("      ready  0");
				else if (processList[i].state == BLOCKED)
					printf("    blocked%3d",processList[i].ioTime);
				else if (processList[i].state == FINISHED)
					printf(" terminated  0");
				}
			printf(".\n");
		}
		// run this cycle-----1. run the CPU 2. run the I/O 3. increment the waiting time
		/* run the CPU */
		if (myCPU.master != NULL){
			myCPU.remainTime -= 1;
			myCPU.master->cpu -= 1;
			myCPU.master->originalBurst -= 1;
			myCPU.master->cpuRun += 1;
		}
		/* run the I/O */
		Node* walker = ioHead;
		for (int i = 0; i < length(&ioHead); i++){
			walker->data->ioTime -= 1;
			walker->data->totalIO += 1;
			walker = walker->next;
		}
		/* increment the waiting time */
		walker = readyHead;
		for( int i = 0; i < length(&readyHead); i++){
			walker->data->waiting += 1;
			walker = walker->next;
		}
		
		// increase the clockTime, check if all processes have finished
		clockTime += 1;

		if (clockTime > 100000 || finishedProcess == processNum){
			if (length(&ioHead) != 0)
				printf("ioHead(%d) is not empty yet.\n",length(&ioHead));
			if (length(&readyHead) != 0)
				printf("readyHead(%d) is not empty yet.\n",length(&readyHead));
			if (length(&waitHead) != 0)
				printf("waitHead(%d) is not empty yet.\n",length(&waitHead));
			break;
		}
		if (length(&ioHead) > 0)
			myCPU.ioTime += 1;
		if (myCPU.master != NULL)
			myCPU.cpuTime += 1;
	}
	
	myCPU.finishTime = clockTime - 1;
	printf("The scheduling algorithm used was Highest Penalty Ratio Next\n\n");
	print_summary(processList,processNum,myCPU);
	fclose(fpt);
}


int main(int argc, const char * argv[]) {

	int verbose = 0;
	char inputFileName[50];
	
	/* ----dealing with read in command line and check if it satisfies our specification---- */
	if (argc == 3){
		if (strcmp(argv[1],"--verbose") == 0 || strcmp(argv[1],"-verbose") == 0){
			verbose = 1;
			strcpy(inputFileName,argv[2]);
		}
		else{
			printf("Command specification wrong. Please check again.\n");
			return 1;
		}
	}
	else if (argc == 2)
		strcpy(inputFileName,argv[1]);
	else{
		printf("Command number wrong. Please check again.\n");
		return 1;
	}
	FILE * fpt = fopen(inputFileName, "r");
	if (fpt == NULL){
		printf("The file %s does not open successfully. Please check again.\n",inputFileName);
		return 1;
	}

	int processNum,A,B,C,M;
	fscanf(fpt,"%d",&processNum);
	Process processListFCFS[processNum];
	Process processListRR[processNum];
	Process processListLCFS[processNum];
	Process processListHPRN[processNum];
	for (int i = 0; i < processNum; i++)
	{ /* initialize the 4 arrays of process. They will be passed to each algorithm respectively */
		fscanf(fpt," (%d %d %d %d)",&A,&B,&C,&M);
		
		processListFCFS[i].id = i;
		processListFCFS[i].oldID = i;
		processListFCFS[i].arrival = A;
		processListFCFS[i].randSize = B;
		processListFCFS[i].cpu = C;
		processListFCFS[i].originalCPU = C;
		processListFCFS[i].ioFactor = M;
		processListFCFS[i].ioRemain = 0;
		processListFCFS[i].ioTime = 0;
		processListFCFS[i].finish = -1;
		processListFCFS[i].originalBurst = 0;
		processListFCFS[i].totalIO = 0;
		processListFCFS[i].waiting = 0;
		processListFCFS[i].state = UNSTARTED;
		processListFCFS[i].preempted = 0;
		processListFCFS[i].cpuRemain = 0;
		processListFCFS[i].T = 0;
		processListFCFS[i].t = 0;
		processListFCFS[i].cpuRun = 0;
		processListFCFS[i].r = 0;
		
		processListRR[i].id = i;
		processListRR[i].oldID = i;
		processListRR[i].arrival = A;
		processListRR[i].randSize = B;
		processListRR[i].cpu = C;
		processListRR[i].originalCPU = C;
		processListRR[i].ioFactor = M;
		processListRR[i].ioRemain = 0;
		processListRR[i].ioTime = 0;
		processListRR[i].finish = -1;
		processListRR[i].originalBurst = 0;
		processListRR[i].totalIO = 0;
		processListRR[i].waiting = 0;
		processListRR[i].state = UNSTARTED;
		processListRR[i].preempted = 0;
		processListRR[i].cpuRemain = 0;
		processListRR[i].T = 0;
		processListRR[i].t = 0;
		processListRR[i].cpuRun = 0;
		processListRR[i].r = 0;
		
		processListLCFS[i].id = i;
		processListLCFS[i].oldID = i;
		processListLCFS[i].arrival = A;
		processListLCFS[i].randSize = B;
		processListLCFS[i].cpu = C;
		processListLCFS[i].originalCPU = C;
		processListLCFS[i].ioFactor = M;
		processListLCFS[i].ioRemain = 0;
		processListLCFS[i].ioTime = 0;
		processListLCFS[i].finish = -1;
		processListLCFS[i].originalBurst = 0;
		processListLCFS[i].totalIO = 0;
		processListLCFS[i].waiting = 0;
		processListLCFS[i].state = UNSTARTED;
		processListLCFS[i].preempted = 0;
		processListLCFS[i].cpuRemain = 0;
		processListLCFS[i].T = 0;
		processListLCFS[i].t = 0;
		processListLCFS[i].cpuRun = 0;
		processListLCFS[i].r = 0;
		
		processListHPRN[i].id = i;
		processListHPRN[i].oldID = i;
		processListHPRN[i].arrival = A;
		processListHPRN[i].randSize = B;
		processListHPRN[i].cpu = C;
		processListHPRN[i].originalCPU = C;
		processListHPRN[i].ioFactor = M;
		processListHPRN[i].ioRemain = 0;
		processListHPRN[i].ioTime = 0;
		processListHPRN[i].finish = -1;
		processListHPRN[i].originalBurst = 0;
		processListHPRN[i].totalIO = 0;
		processListHPRN[i].waiting = 0;
		processListHPRN[i].state = UNSTARTED;
		processListHPRN[i].preempted = 0;
		processListHPRN[i].cpuRemain = 0;
		processListHPRN[i].T = 0;
		processListHPRN[i].t = 0;
		processListHPRN[i].cpuRun = 0;
		processListHPRN[i].r = 0;
	}
	fclose(fpt);
	/* ----------Start 4 scheduling algorithms-------------*/
	printf("\n------------------------------------------\n");
	FCFS(processListFCFS, processNum, verbose); // First-Come-First-Serve algorithm
	printf("\n------------------------------------------\n");
				 
	RR(processListRR, processNum, verbose); // Round-Robin quantum=2 algorithm
	printf("\n------------------------------------------\n");
				 
	LCFS(processListLCFS, processNum, verbose); // Last-Come-First-Serve algorithm
	printf("\n------------------------------------------\n");
				 
	HPRN(processListHPRN, processNum, verbose); // Highest-Penalty-Ratio-Next algorithm
	printf("\n------------------------------------------\n");
	return 0;
}
