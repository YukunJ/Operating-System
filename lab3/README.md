# Spring 2020 Operating System
### Lab3: Optimistic Resource Allocator/Banker's Algorithms
### Name: Yukun Jiang
### Net id: jy2363
### Email: jy2363@nyu.edu

## Hello, welcome to my documentation. It's gonna be brief, please do read them.


### 1. How to run my code?
After you have uploaded the code file “banker.cpp", all input files to the nyu crackle system. Type in the command window:
g++ -std=c++11 -o banker banker.cpp
then Type in:
./banker input-01
Here you go!
The output order is : Optimistic Resource Allocator ----->  Banker's 

### 2. Input specification & Important assumptions
1. for input specification, my system accept input scattered on different lines and dispersed Instructions for different process. So it's robust.
2. Important assumptions in "initialization" for both Optimistic Resource Allocator & Banker's. I assume (and it's true in all input-files) that, even if a process doesn't claim for a resource, it still write "claim 0" instruction.
    
    For example, there are 2 resources A and B. The process 1 claim to use only 1 unit of A. But I need it to write:
    initiate 1 0 A 1
    initiate 1 0 B 0
    
    instead of only:
    initiate 1 0 A 1
    
    This helps to keep a synchronous pace of initialization of process. In another word, all processes hopefully would enter the running list at the same cycle adter initialization. 

### 3. Design Pattern
I adopt OOP sysle programming using C++ in this lab. There are two main classes: Process & Instruction. The Instruction is nested in Process by a vector<Instruction> to record all the instruction for a specific process.  There are detailed comments for each class and function in the source code. So I will just briefly explain the high-level procedure for my algorithm. 

For Optimistic Resource Allocator:
Repeat until completion :
1. Finish Initialization (since it doesn't care about claim. Guaranteed success)
2. Check blocking list, to see if any blocked request could be satisified
3. Execute instructions for each process in the running list
4. Check for deadlocks, if so, abort the lowest index process to release resources
5. increment cycle and add wait time for process in blocking list

For Banker's algorithm:
Repeat until completion:
1. Try Initialization ( if claim too much, just abort it)
2. Check blocking list, to see if any blocked request could be satisfied based on whether safe state
3. Execute instructions for each process in the running list (if request exceeds claim, abort it)
4. increment cycle and add wait time for process in blocking list

### 4. Contact Info
if you run into any trouble understanding my design or compiling the code, please contact me by jy2363@nyu.edu. Reply is guaranteed within 1/2 hour.

