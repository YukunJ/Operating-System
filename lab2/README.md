
# Spring 2020 Operating System
### Lab1: Scheduling Algorithms
### Name: Yukun Jiang
### Net id: jy2363
### Email: jy2363@nyu.edu

## Hello, welcome to my documentation. It's gonna be brief, please do read them.


### 1. How to run my code?
After you have uploaded the code file, input file and the random-numbers.txt to crackle. Make sure they are in the same folder.
Then, you type in the terminal "gcc -std=c99 os_lab2_scheduling.c", it's important for you to speicfy c99 version, because in the code there are a lot of place using " for ( int i = 0; .......;......) " doing loop variable declaration inside the for loop statement. If you don't specify c99 version, it will generate some warnings.
Then, say you want to execute for input-1, you type in the terminal "./a.out input-1" or "./a.out --verbose input-1", depending on whether you want to see the detailed cycle information. There will be 4 sections output, separated by "--------------------------" one after another. The output order is: FCFS->RR->LCFS->HPRN

### 2. Input specification
There are mainly 2 points to be aware of to make sure the input is valid.
1. The buffer I use to store the input file name is 50-character long. So you should not enter a file name longer than 50 characters ( you probably will not). It would generally be fine.
2. Each quadruple should be closed with parenthesis. and more importantly, each quadruple should bd precede with a space before anything preceding it. This is mainlu due to the way I extract quadruple in the code. 
	I do "fscanf(fpt," (%d %d %d %d)",&A,&B,&C,&M);" Notice there is a empty space before the (%d %d %d %d).
	I don't necessarily require all the input on 1 single line. But I need each quadruple to be 1 single line, meaning you should not split a quadruple on 2 lines. Because the function "fscanf" require exact match pattern, and I don't have "\n" in that pattern. In brieft, if you follow the 7 sample inputs structures, it will work find and properly.
	
### 3. My design pattern
I mainly mimic an OOP style programming despite the fact that I am in the C world. I use "struct Process" to encapsulate all the information about one process in a struct. In the main function, we allocate 4 array of processes, pass each one to corresponding algorithm functions, since array are passed by reference, each algoirthm should have a distinct array without  interference each other. Also there is a "struct CPU" which mimic a single-core CPU.

I self-implement a single-linked list, with its data membership being a pointer to a process, so we can modify the process information accordingly. Also I implement several comparison functions to be passed into C standard "qsort" function. These functions helps to sort either by process's arrival time, or order in the input file, or I/O remaining time, or Penalty ratio.

In each algorithm, there exist 3 Linked list permanently throughout the algorithm execution:
	readyHead: the linked-list of process that are ready to be put to CPU running.
	ioHead: the linked-list of process that are doing I/O, namely blocked
	waitingHead: the linked-list of process that has not arrival yet.
	myCPU: a single-core CPU, recording which process is running or no process is running
	
In each algorithm, the general process is the same: we increment the time by 1 second, check if any process has arrived, if any process has finished I/O, we sort the readyList according to a certain criteria needed by the algorithm. Then we check the "myCPU", if no process is running, try to drag one from readylist to run. If there is a process running, check if it has finished/ need blocking / run out of quantum. After all the checkings, we let the time increment by 1.
	
### 4. Contact Info
if you run into any trouble understanding my design or compiling the code, please contact me by jy2363@nyu.edu. Reply is guaranteed within 1/2 hour.
